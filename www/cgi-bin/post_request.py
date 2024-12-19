#!/usr/bin/env python3

import cgi
import cgitb
import os
import sys

# Enable debugging
cgitb.enable()

def main():
    # Ensure the script is being called with a POST request
    if os.environ['REQUEST_METHOD'] != 'POST':
        print("Status: 405 Method Not Allowed")
        print("Content-Type: text/html")
        print()
        print("<html><body><h1>405 Method Not Allowed</h1></body></html>")
        return

    # Read the POST data
    content_length = int(os.environ['CONTENT_LENGTH'])
    post_data = sys.stdin.read(content_length)

    # Process the POST data (for example, parse it as form data)
    form = cgi.FieldStorage(fp=sys.stdin, environ=os.environ, keep_blank_values=True)

    # Generate the response
    print("Content-Type: text/html")
    print()
    print("<html><body>")
    print("<h1>POST Data Received</h1>")
    print("<pre>")
    print(cgi.escape(post_data))
    print("</pre>")
    print("</body></html>")

if __name__ == "__main__":
    main()