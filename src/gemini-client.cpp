#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/time.h>
#include "../include/gemini-client.hpp"
#include "../include/site.hpp"
#include "../include/shared.hpp"
#include <chrono>
#include "../include/utils.hpp"
#include "../include/errors.hpp"
#include <openssl/ssl.h>
#include <string>
#include <poll.h>

Site* GeminiClient::getNetworkedSite(Link link, std::string crtPath, std::string keyPath) {

    if (!isSendableIfGeminiUrl(link.getLinkDestination())) {
        return nullptr;
    }

    std::string host = link.getLinkDestination().get_host();
    std::string req  = link.getLinkDestination().to_string() + "\r\n";
    std::string conn = host + ":1965";

    if(link.getLinkDestination().get_port()) {
        conn = host + ":"  + std::to_string(link.getLinkDestination().get_port());
    }

    std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> ctx(SSL_CTX_new(TLS_client_method()), SSL_CTX_free);

    if (!ctx) {
        return nullptr;
    }

    if(crtPath != "" && keyPath != "") {
        if (SSL_CTX_use_certificate_file(ctx.get(), crtPath.c_str(), SSL_FILETYPE_PEM) <= 0 || SSL_CTX_use_PrivateKey_file(ctx.get(), keyPath.c_str(), SSL_FILETYPE_PEM) <= 0 || !SSL_CTX_check_private_key(ctx.get())) {
            return nullptr;
        }
    }

    std::unique_ptr<BIO, decltype(&BIO_free_all)> bio(BIO_new_ssl_connect(ctx.get()), BIO_free_all);

    if (!bio) {
        return nullptr;
    } 

    SSL* ssl;
    BIO_get_ssl(bio.get(), &ssl);
    SSL_set_tlsext_host_name(ssl, host.c_str());
    BIO_set_conn_hostname(bio.get(), conn.c_str());
    BIO_set_nbio(bio.get(), 1);

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);

    while (BIO_do_connect(bio.get()) <= 0) {

        int fd = -1;

        long ms = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now()).count();
        pollfd p{};

        // yikes.
        if (!BIO_should_retry(bio.get()) || BIO_get_fd(bio.get(), &fd) < 0 || fd < 0 || ms <= 0 || (p = {fd, short(BIO_should_read(bio.get()) ? POLLIN : POLLOUT), 0}, poll(&p, 1, int(ms)) <= 0)) {
            return nullptr;
        }
    }

    // timeout, 5 seconds
    int fd = -1;
    if (BIO_get_fd(bio.get(), &fd) >= 0 && fd >= 0) {
        BIO_socket_nbio(fd, 0);
        timeval tv{5, 0};
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);
    }

    BIO_write(bio.get(), req.data(), (int)req.size());

    std::string response;
    char buf[4096];
    int n;

    int sizeRemaining = RESPONSE_SIZE_LIMIT_MB * 1024 * 1024;

    while ((n = BIO_read(bio.get(), buf, sizeof buf)) > 0) {
        response.append(buf, n);

        sizeRemaining -= n;
        if(sizeRemaining <= 0) {
            break;
        }
    }

    bio.reset();

    std::size_t nl = response.find('\n');
    if (nl == std::string::npos) {
        return nullptr;
    }

    std::size_t end = nl;
    if (end > 0 && response[end - 1] == '\r') {
        end -= 1;
    }

    // TODO: should we track when these are truncated? We have a variable for that
    // but nothign about it in our site. 

    std::string status = response.substr(0, end);
    std::string body   = response.substr(nl + 1);

    return new Site(status, body);
}


Site* GeminiClient::fetchSite(Link link, std::string crtPath, std::string keyPath) {
    std::string destination = link.getLinkDestination().to_string();

    if(isPrefixed(destination, "gemini://")) {
        try {
            return getNetworkedSite(link, crtPath, keyPath);
        } catch (...) {
            auto* unreach = new Site{"41 server unreachable", ""};
            unreach->setUnreachable();
            return unreach;
        }
    } else if (isPrefixed(destination, "file://")){ // TODO: This seems wrong; it should probably be fullpath with that prefix.

        // TODO: this is messy and perhaps not necessary
        
        std::string rest = destination.substr(7);
        std::string host;
        std::string path;

        std::size_t slash = rest.find('/');
        if (slash != std::string::npos) {
            host = rest.substr(0, slash);
            path = rest.substr(slash);
        } else {
            host = rest;
            path = "";
        }

        if (host.empty()) {
            host = "localhost";
        }

        std::string fileStr = "";

        std::string fsPath;

        if(host == "localhost") {
            fsPath = path;
        }  else {
            throw std::invalid_argument("The requested file appears to exist on another system.");
        }

        try {
            fileStr = readFileToString(fsPath);
        } catch (FileReadError& e ) {
            return new Site {"51 file not found", ""};
        }
        // TODO: how should I discern file types?
        return new Site {"20 text/gemini", fileStr};
    } else if(isPrefixed(destination, "about://")){
        return new Site {"20 text/gemini", getNewTab()};
    } else {
        throw NotImplemented();
    }

    throw NotImplemented();
}
