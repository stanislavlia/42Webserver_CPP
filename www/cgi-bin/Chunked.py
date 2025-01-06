# # #!/usr/bin/env python3
# # import os
# # import sys
# # import html
# # from urllib.parse import parse_qs

# # COMMENTS_FILE = "comments.txt"
# # COMMENTS_HTML_FILE = "www/static/comments.html"

# # def read_request_body():
# #     content_length = int(os.environ.get('CONTENT_LENGTH', 0))
# #     return sys.stdin.read(content_length)

# # def save_comment(comment):
# #     with open(COMMENTS_FILE, 'a') as file:
# #         file.write(comment + '\n')

# # def load_comments():
# #     if os.path.exists(COMMENTS_FILE):
# #         with open(COMMENTS_FILE, 'r') as file:
# #             return file.readlines()
# #     return []

# # def parse_comment(comment):
# #     parsed = parse_qs(comment)
# #     username = parsed.get('username', [''])[0]
# #     comment_text = parsed.get('comment', [''])[0]
# #     return f"{html.escape(username)}: {html.escape(comment_text)}"

# # def generate_html(comments):
# #     comments_html = ''.join(f"<p>{comment}</p>\n" for comment in comments)
# #     return f"""<!DOCTYPE html>
# # <html lang="en">
# # <head>
# #     <meta charset="UTF-8">
# #     <meta name="viewport" content="width=device-width, initial-scale=1.0">
# #     <title>Comments</title>
# #     <style>
# #         body {{
# #             background-color: #121212;
# #             color: white;
# #             font-family: Arial, sans-serif;
# #             text-align: center;
# #             margin: 0;
# #             padding: 0;
# #         }}
# #         .content {{
# #             margin-top: 20px;
# #             padding: 20px;
# #             background-color: #1e1e1e;
# #             border-radius: 8px;
# #             box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
# #             display: inline-block;
# #             text-align: left;
# #             max-width: 600px;
# #         }}
# #         p {{
# #             white-space: pre-wrap;
# #             word-wrap: break-word;
# #         }}
# #     </style>
# # </head>
# # <body>
# #     <h1>Comments</h1>
# #     <div class="content">
# #         {comments_html}
# #     </div>
# # </body>
# # </html>
# # """

# # def update_html_file():
# #     comments = load_comments()
# #     parsed_comments = [parse_comment(comment) for comment in comments]
# #     html_content = generate_html(parsed_comments)
# #     with open(COMMENTS_HTML_FILE, 'w') as file:
# #         file.write(html_content)

# # def main():
# #     try:
# #         # Read the request body
# #         body_content = read_request_body()
        
# #         # Save the new comment
# #         save_comment(body_content)
        
# #         # Update the HTML file with all comments
# #         update_html_file()
        
# #         # Load the updated HTML content
# #         with open(COMMENTS_HTML_FILE, 'r') as file:
# #             html_response = file.read()
        
# #         # Print the HTML response
# #         print(html_response)
# #     except Exception as e:
# #         print("Status: 500 Internal Server Error")
# #         print("Content-Type: text/plain")
# #         print()
# #         print(f"An error occurred: {e}")

# # if __name__ == "__main__":
# #     main()

# #!/usr/bin/env python3
# import os
# import sys
# import html
# from urllib.parse import parse_qs

# COMMENTS_FILE = "comments.txt"

# def read_request_body():
#     content_length = int(os.environ.get('CONTENT_LENGTH', 0))
#     return sys.stdin.read(content_length)

# def save_comment(comment):
#     with open(COMMENTS_FILE, 'a') as file:
#         file.write(comment + '\n')

# def load_comments():
#     if os.path.exists(COMMENTS_FILE):
#         with open(COMMENTS_FILE, 'r') as file:
#             return file.readlines()
#     return []

# def parse_comment(comment):
#     parsed = parse_qs(comment)
#     username = parsed.get('username', [''])[0]
#     comment_text = parsed.get('comment', [''])[0]
#     return f"{html.escape(username)}: {html.escape(comment_text)}"

# def generate_html(comments):
#     comments_html = ''.join(f"<p>{comment}</p>\n" for comment in comments)
#     return f"""<!DOCTYPE html>
# <html lang="en">
# <head>
#     <meta charset="UTF-8">
#     <meta name="viewport" content="width=device-width, initial-scale=1.0">
#     <title>Comments</title>
#     <style>
#         body {{
#             background-color: #121212;
#             color: white;
#             font-family: Arial, sans-serif;
#             text-align: center;
#             margin: 0;
#             padding: 0;
#         }}
#         .content {{
#             margin-top: 20px;
#             padding: 20px;
#             background-color: #1e1e1e;
#             border-radius: 8px;
#             box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
#             display: inline-block;
#             text-align: left;
#             max-width: 600px;
#         }}
#         p {{
#             white-space: pre-wrap;
#             word-wrap: break-word;
#         }}
#     </style>
# </head>
# <body>
#     <h1>Comments</h1>
#     <div class="content">
#         {comments_html}
#     </div>
# </body>
# </html>
# """

# def main():
#     try:
#         # Print the HTTP header
#         print("Content-Type: text/html\n")

#         # Read the request body
#         body_content = read_request_body()
        
#         # Save the new comment
#         save_comment(body_content)
        
#         # Load all comments
#         comments = load_comments()
        
#         # Parse and generate HTML for comments
#         parsed_comments = [parse_comment(comment) for comment in comments]
#         html_response = generate_html(parsed_comments)
        
#         # Print the HTML response
#         print(html_response)
#     except Exception as e:
#         print("Status: 500 Internal Server Error")
#         print("Content-Type: text/plain")
#         print()
#         print(f"An error occurred: {e}")

# if __name__ == "__main__":
#     main()

# #!/usr/bin/env python3
# import os
# import sys
# import html
# from urllib.parse import parse_qs

# COMMENTS_FILE = "comments.txt"

# def read_request_body():
#     content_length = int(os.environ.get('CONTENT_LENGTH', 0))
#     return sys.stdin.read(content_length)

# def save_comment(comment):
#     with open(COMMENTS_FILE, 'a') as file:
#         file.write(comment + '\n')

# def load_comments():
#     if os.path.exists(COMMENTS_FILE):
#         with open(COMMENTS_FILE, 'r') as file:
#             return file.readlines()
#     return []

# def parse_comment(comment):
#     parsed = parse_qs(comment)
#     username = parsed.get('username', [''])[0]
#     comment_text = parsed.get('comment', [''])[0]
#     return f"{html.escape(username)}: {html.escape(comment_text)}"

# def main():
#     try:
#         # Print the HTTP header
#         print("Content-Type: text/html\n")

#         # Read the request body
#         body_content = read_request_body()
        
#         # Save the new comment
#         save_comment(body_content)
        
#         # Load all comments
#         comments = load_comments()
        
#         # Parse and generate HTML for comments
#         parsed_comments = [parse_comment(comment) for comment in comments]
        
#         # Generate and print the HTML response
#         print("<html>")
#         print("<head>")
#         print("<title>Comments</title>")
#         print("</head>")
#         print("<body>")
#         print("<h1>Comments</h1>")
#         print('<div style="background-color: #1e1e1e; color: white; padding: 20px; border-radius: 8px; max-width: 600px; margin: 20px auto;">')
#         for comment in parsed_comments:
#             print(f"<p>{comment}</p>")
#         print("</div>")
#         print("</body>")
#         print("</html>")
#     except Exception as e:
#         print("Status: 500 Internal Server Error")
#         print("Content-Type: text/plain")
#         print()
#         print(f"An error occurred: {e}")

# if __name__ == "__main__":
#     main()

#!/usr/bin/env python3
import os
import sys
import html
from urllib.parse import parse_qs

COMMENTS_FILE = "comments.txt"
COMMENTS_HTML_FILE = "www/static/comments.html"

def read_request_body():
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    return sys.stdin.read(content_length)

def save_comment(comment):
    with open(COMMENTS_FILE, 'a') as file:
        file.write(comment + '\n')

def load_comments():
    if os.path.exists(COMMENTS_FILE):
        with open(COMMENTS_FILE, 'r') as file:
            return file.readlines()
    return []

def parse_comment(comment):
    parsed = parse_qs(comment)
    username = parsed.get('username', [''])[0]
    comment_text = parsed.get('comment', [''])[0]
    return f"{html.escape(username)}: {html.escape(comment_text)}"

def generate_html(comments):
    comments_html = ''.join(f"<p>{comment}</p>\n" for comment in comments)
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Comments</title>
    <style>
        body {{
            background-color: #121212;
            color: white;
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 0;
        }}
        .content {{
            margin-top: 20px;
            padding: 20px;
            background-color: #1e1e1e;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            display: inline-block;
            text-align: left;
            max-width: 600px;
        }}
        p {{
            white-space: pre-wrap;
            word-wrap: break-word;
        }}
    </style>
</head>
<body>
    <h1>Comments</h1>
    <div class="content">
        {comments_html}
    </div>
</body>
</html>
"""

def update_html_file():
    comments = load_comments()
    parsed_comments = [parse_comment(comment) for comment in comments]
    html_content = generate_html(parsed_comments)
    with open(COMMENTS_HTML_FILE, 'w') as file:
        file.write(html_content)

def main():
        # Read the request body
        body_content = read_request_body()
        
        # Save the new comment
        save_comment(body_content)
        
        # Update the HTML file with all comments
        update_html_file()
        
        # Print nothing to keep the user on the home page
        print("<html>")
        print("<head>")
        print("<title>Form Submission Result</title>")
        print("</head>")
        print("<body>")
        print("<h1>Comment posted</h1>")
        print("</body>")
        print("</html>")

if __name__ == "__main__":
    main()