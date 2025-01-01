#!/usr/bin/ python3

import os
import sys

print("Content-Type: text/plain")
print("Content-Disposition: attachment; filename=test_cgi.txt")
print()
print("CGI script is working")
print("Request Method:", os.environ.get("REQUEST_METHOD"))
print("Query String:", os.environ.get("QUERY_STRING"))
print("Content Length:", os.environ.get("CONTENT_LENGTH"))
print("Content Type:", os.environ.get("CONTENT_TYPE"))