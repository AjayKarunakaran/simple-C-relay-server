
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libwebsockets.h>
void *connections[1024];
uint8_t connection_count = 0; //overflow will just overwrite older connections instead of crashing. 
//small side effect is that old connections will be ghosts, sending data but not receiving any^
// Define our protocol callback handler
static int callback_game(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            lwsl_user("A web browser client connected.\n");
            connections[connection_count++] = wsi; // add new connection wsi = websocket instance
            break;

        case LWS_CALLBACK_RECEIVE:
            // 'in' contains the message from the browser, 'len' is its length
            lwsl_user("Received from client: %s\n", (char *)in);

            // Prepare a response message. 
            // NOTE: LWS_PRE bytes must be preserved at the start of the buffer for protocol headers.
            {
                unsigned char buf[LWS_PRE + 4096];


                // Copy message after the LWS_PRE padding

                memcpy(&buf[LWS_PRE], in, len);
                // Send text frame back to the browser
                for (int i = 0; i < connection_count; i++) {
                    if (connections[i] != wsi) {// don't send data back to the user who sent it
                        lws_write(connections[i], &buf[LWS_PRE], len, LWS_WRITE_TEXT);
                    }

                }
            }
            break;

        case LWS_CALLBACK_CLOSED:
            lwsl_user("Client disconnected.\n");
            for (int i = 0; i < connection_count; i++) {
                if (connections[i] == wsi) {
                    // Shift remaining connections down
                    for (int j = i; j < connection_count - 1; j++) {
                        connections[j] = connections[j + 1];
                    }
                    break;
                }
            }
            connection_count--;
            break;

        default:
            break;
    }
    return 0;
}

// protocol, which must be used client side also
static struct lws_protocols protocols[] = {
    {
        "game-protocol",  // Sub-protocol name 
        callback_game,    // Callback function
        0,                // Per User-session memory size used in callback *user for id or user specific information
        4096,              // maximum Receive buffer size
        0,                // ID number (not used)
        NULL,             // pointer to user-defined data
        0                 // Send size, default = receive buffer size
    },
    { NULL, NULL, 0, 0, 0, NULL, 0 }// tells lws how many protocols there are

};

int main(int argc, char **argv) {
    struct lws_context_creation_info info;
    struct lws_context *context;
    
    memset(&info, 0, sizeof(info));
    info.port = 8080;                   // Port to listen on must be the same as the client side
    info.protocols = protocols;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT; //force ssl init
    info.ssl_cert_filepath = "/etc/letsencrypt/live/game.ajaykarunakaran.com/fullchain.pem";
    info.ssl_private_key_filepath = "/etc/letsencrypt/live/game.ajaykarunakaran.com/privkey.pem";
    info.gid = -1;
    info.uid = -1;

    // Create the libwebsockets context
    context = lws_create_context(&info);
    if (!context) {
        lwsl_err("Failed to initialize libwebsockets context\n");
        return 1;
    }

    lwsl_user("WebSocket game server started on port 8080...\n");

    // Event loop: service incoming network events every 50ms
    while (1) {
        lws_service(context, 50);//poll for new connections every 50 ms, connections already established will be handled by the callback function
    }

    lws_context_destroy(context);
    return 0;
}
