# Spotify extension

The Spotify extension for Albert launcher allows you to search
tracks on Spotify and play them immediately or add them to the
queue. It also allows you to choose the Spotify client, where
to play the track.

The extension uses the Spotify Web API.

For the proper
functionality of extension, **Spotify premium is required**.

![Spotify extension](https://i.imgur.com/CoE2C5i.png)

## Web API connection

### 1. Get your Client ID and Client Secret

Visit: https://developer.spotify.com/dashboard/applications and log
in with your Spotify account.

Click on the button **Create an app**
and fill the form. You can use for example name "Albert" and
description "Spotify extension for Albert launcher".

Once you
click on **Create**, your new application window will appear. You
can copy your **Client ID** and show **Client Secret**.
Both are 32-character strings.

Click on **Edit settings** and add new **Redirect URI**. It doesn't
have to exist. In this example, I will use: `https://nonexistent-uri.net/`

### 2. Get `code` parameter

Open your browser and visit: https://accounts.spotify.com/cs/authorize?response_type=code&client_id=[[client_id]]&scope=user-modify-playback-state%20user-read-playback-state&redirect_uri=https://nonexistent-uri.net/

You have to replace `[[client_id]]` with your actual **Client ID**.

When you press enter, you will get redirected to
`https://nonexistent-uri.net/` with `code` in URL parameters.
Copy that string and note it down for the next usage.

### 3. Get your Refresh Token

I will use `curl` for this last step. Replace or export all variables and run this command:

```
curl -d client_id=$CLIENT_ID -d client_secret=$CLIENT_SECRET -d grant_type=authorization_code -d code=$CODE -d redirect_uri=https://nonexistent-uri.net/ https://accounts.spotify.com/api/token
```

Use your Client ID, Client Secret and `code` from the previous step.

It will send POST request and return JSON in the answer.
You can finally get your **Refresh Token**.

<hr>

The whole process is also similarly described
[here](https://benwiz.com/blog/create-spotify-refresh-token/).