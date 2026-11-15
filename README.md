# simple-C-relay-server
establishes websocket connections from a client and relays all messages to other connected clients. for use in VPS or home server.
requires libwebsockets library for compilation

simplest way to run this for free 
1. get a sub domain from duck dns or one of your choosing from any provider
2. create an A record under that domain
3. get a VPS, google cloud offers a VM for free so long as you
   1. choose a free non-preemptible e2 micro instance
   2. debian is a good choice
   3. disable snapshots and backups
   4. change storage from balanced pd to standard persistent disk must be less than 30gb
   5. change networking from premium to standard(standard gives 200gb outbound free)
   6. ensure both internal and external ip is ephemeral and not static.
   7. you will also need to enable http and https traffic
   you can also do the same with oracle cloud but the setup is more complicated however they do give you 10TB outbound traffic for free

4. ssh into the server and install DDclient, certbot, libwebsockets-dev and your choice of c compiler
5. setup dd client, the config settings is wrong so change them. i used cloudflare so here is the example, with api token
   protocol=cloudflare,
   zone=domain.com,
   use=web web=ipify-ipv4,
   login=token,
   passsword="api-token",
   subdomain.domain.com

   the login is literaly just the word token and then put the token in the password space
   ddclient automatically changes the A record listed in your dns provider to point to your server hosted in the cloud

6. run ddclient as a systemd service and run certbot
   sudo systemctl enable ddclient
   sudo certbot certonly --standalone -d yourdomain.com

   now you have subdomain.domain.com which is pointing to your server with ssl records in etc/letsencrypt/live/
7. write or compile the script put it in the bin directory and run it automatically with systemd

yay you have a server repeater for free
