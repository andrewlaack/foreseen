#!/usr/bin/env python3
import random
import os

query = os.environ.get("QUERY_STRING", "")

if not query:
    print(f"10 enter some info", end="\r\n")
else:
    print(f"20 text/gemini", end="\r\n")
    print("Hey, you entered: " + query)

