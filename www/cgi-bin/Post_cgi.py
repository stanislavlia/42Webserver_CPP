#!/usr/bin/env python3

import os
import sys
import urllib.parse

def parse_post_data():
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    post_data = sys.stdin.read(content_length)
    return urllib.parse.parse_qs(post_data)

def main():

    # Parse the POST data
    post_params = parse_post_data()

    # Get the values from the POST data
    username = post_params.get("username", [""])[0]
    email = post_params.get("email", [""])[0]

    # Generate the HTML response
    print("<html>")
    print("<head>")
    print("<title>Form Submission Result</title>")
    print("</head>")
    print("<body>")
    print("<h1>Form Submission Result</h1>")

    if not username or not email:
        print("<p>Error: Both username and email are required!</p>")
    else:
        print(f"<p>Username: {username}</p>")
        print(f"<p>Email: {email}</p>")

    print("</body>")
    print("</html>")

if __name__ == "__main__":
    main()