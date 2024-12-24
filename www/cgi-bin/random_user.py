#!/usr/bin/env python3

import sys
import argparse
import requests

def fetch_random_users(count):
    """
    Fetch multiple random users from the API.
    """
    if count > 10 or count < 1:
        print("Error: Number of users must be between 1 and 10.", file=sys.stderr)
        sys.exit(1)
    
    url = f"https://randomuser.me/api/"
    params = {"results": count}
    try:
        response = requests.get(url, params=params)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Error fetching user data: {e}", file=sys.stderr)
        sys.exit(1)

def generate_html(user_data):
    """
    Generate HTML content listing all fetched user profiles.
    """
    profiles_html = ""
    
    for user in user_data['results']:
        name = f"{user['name']['title']} {user['name']['first']} {user['name']['last']}"
        gender = user['gender'].capitalize()
        email = user['email']
        dob = user['dob']['date'].split("T")[0]  # Extract only the date part
        age = user['dob']['age']
        phone = user['phone']
        cell = user['cell']
        picture = user['picture']['large']
        country = user['location']['country']
        city = user['location']['city']
        timezone = user['location']['timezone']['description']
        
        profiles_html += f"""
        <div class="profile">
            <img src="{picture}" alt="User Picture">
            <h2>{name}</h2>
            <p><strong>Gender:</strong> {gender}</p>
            <p><strong>Email:</strong> {email}</p>
            <p><strong>Location:</strong> {city}, {country}</p>
            <p><strong>Timezone:</strong> {timezone}</p>
            <p><strong>Date of Birth:</strong> {dob} (Age: {age})</p>
            <p><strong>Phone:</strong> {phone}</p>
            <p><strong>Cell:</strong> {cell}</p>
        </div>
        """

    html_content = f"""
    <html>
    <head>
        <title>Random User Profiles</title>
        <style>
            body {{
                font-family: Arial, sans-serif;
                background-color: #f9f9f9;
                margin: 0;
                padding: 0;
                display: flex;
                flex-direction: column;
                align-items: center;
            }}
            .profile {{
                background: white;
                border-radius: 10px;
                padding: 20px;
                width: 400px;
                margin: 15px;
                text-align: center;
                box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            }}
            .profile img {{
                border-radius: 50%;
                width: 150px;
                height: 150px;
                margin-bottom: 15px;
            }}
            h1 {{
                text-align: center;
                margin: 20px 0;
            }}
        </style>
    </head>
    <body>
        <h1>Random User Profiles</h1>
        {profiles_html}
    </body>
    </html>
    """
    return html_content

def main():
    """
    Main function to handle CLI arguments and generate HTML.
    """
    parser = argparse.ArgumentParser(description="Fetch random user profiles and generate HTML.")
    parser.add_argument("n", type=int, help="Number of user profiles to fetch (1-10).")
    args = parser.parse_args()

    # Fetch user data
    user_data = fetch_random_users(args.n)

    # Generate and print HTML
    print(generate_html(user_data))

if __name__ == "__main__":
    main()