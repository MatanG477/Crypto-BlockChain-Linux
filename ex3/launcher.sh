#!/bin/bash

NUM_DECRYPTERS=${1:-3}

# 0. התקנת mta-utils-dev-x86_64.deb (אם נדרש; ללא שינוי לשאר הסקריפט)
PKG_PATH=""
if [ -f "./ex3/mta-utils-dev-x86_64.deb" ]; then
  PKG_PATH="./ex3/mta-utils-dev-x86_64.deb"
elif [ -f "./mta-utils-dev-x86_64.deb" ]; then
  PKG_PATH="./mta-utils-dev-x86_64.deb"
fi

if [ -n "$PKG_PATH" ]; then
  if ! dpkg -l | grep -E '^ii\s+.*mta-utils' >/dev/null 2>&1; then
    echo "Installing mta-utils from $PKG_PATH ..."
    sudo dpkg -i "$PKG_PATH" || { sudo apt-get -f install -y && sudo dpkg -i "$PKG_PATH"; }
  fi
else
  echo "WARNING: mta-utils-dev-x86_64.deb not found in ./ex3/ or repo root; continuing..."
fi

# 1. מחיקת כל הפייפים הישנים וקבצים זמניים
sudo find /mnt/mta/ -type p -delete 2>/dev/null
sudo rm -f /mnt/mta/decrypter_pipe_* /mnt/mta/server_pipe

# 2. יצירת קובץ קונפיגורציה (אם לא קיים)
echo "PASSWORD_LENGTH=24" | sudo tee /mnt/mta/mtacrypt.conf > /dev/null
sudo chmod 666 /mnt/mta/mtacrypt.conf

# 3. תיקון הרשאות לכל הקבצים והפייפים בתיקייה
sudo chmod 666 /mnt/mta/* 2>/dev/null
sudo find /mnt/mta -type p -exec chmod 666 {} \; 2>/dev/null
sudo chmod 777 /mnt/mta

# 4. עצירת קונטיינרים קיימים (לא מוחק images)
sudo docker rm -f encrypter &>/dev/null
for i in $(seq 1 $NUM_DECRYPTERS); do
    sudo docker rm -f decrypter$i &>/dev/null
done

# 5. משיכת התמונות מדוקר האב (Docker Hub)
sudo docker pull matangur/mta-encrypter:latest
sudo docker pull matangur/mta-decrypter:latest

# 6. הפעלת האנקריפטר
ENCRYPTER_ID=$(sudo docker run -d \
    --name encrypter \
    -v /mnt/mta:/mnt/mta \
    -v /var/log/encrypter:/var/log \
    matangur/mta-encrypter:latest)
echo "Encrypter ${ENCRYPTER_ID} starting..."
sleep 3

# 7. הפעלת הדקריפטרים
for i in $(seq 1 $NUM_DECRYPTERS); do
    DECRYPTER_ID=$(sudo docker run -d \
        --name decrypter$i \
        -v /mnt/mta:/mnt/mta \
        -v /var/log/decrypter$i:/var/log \
        matangur/mta-decrypter:latest)
    echo "Decrypter #${i} (${DECRYPTER_ID}) starting..."
done

echo "System initialized with $NUM_DECRYPTERS decrypters"

# 8. ניקוי dangling images (אופציונלי)
sudo docker image prune -f > /dev/null 2>&1
