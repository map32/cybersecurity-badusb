docker rm badusb
docker build -t cyb .
docker run --rm --name badusb -d -i -t -v %cd%:/home cyb
docker exec -it badusb bash