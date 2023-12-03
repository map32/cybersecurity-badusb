docker rm badusb_server
docker build -t serv .
docker run --rm --name badusb_server -i -t -v %cd%:/home -p 5000:5000 serv