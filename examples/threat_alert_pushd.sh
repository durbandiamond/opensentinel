#!/bin/bash
echo "OpenSentinel got threat alert from $1.";
echo "Taking action...";

curl -X POST -s --form-string "app_key=YOUR_APP_KEY" --form-string "app_secret=YOUR_APP_SECRET" --form-string "target_type=app" --form-string "content=Threat detected from  $1" https://api.pushed.co/1/push

echo "Action taken.";
