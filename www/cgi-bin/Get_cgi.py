#!/usr/bin python3

import os
import urllib.parse

def parse_query_string():
    query_string = os.environ.get('QUERY_STRING', '')
    return urllib.parse.parse_qs(query_string)

def main():

    # Parse the query string
    query_params = parse_query_string()


    # Get the values from the query string
    username = query_params.get("username", [""])[0]
    email = query_params.get("email", [""])[0]

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