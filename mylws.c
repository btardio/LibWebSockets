

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include <libwebsockets.h>


enum demo_protocols {
        /* always first */
        PROTOCOL_HTTP = 0,

        PROTOCOL_DUMB_INCREMENT,
        PROTOCOL_LWS_MIRROR,
        PROTOCOL_LWS_ECHOGEN,
        PROTOCOL_LWS_STATUS,

        /* always last */
        DEMO_PROTOCOL_COUNT
};

/* list of supported protocols and callbacks */

static struct lws_protocols protocols[] = {
        /* first protocol must always be HTTP handler */

        {
                "http-only",            /* name */
                callback_http,          /* callback */
                sizeof (struct per_session_data__http), /* per_session_data_size */
                0,                      /* max frame size / rx buffer */
        },
        {
                "dumb-increment-protocol",
                callback_dumb_increment,
                sizeof(struct per_session_data__dumb_increment),
                10, /* rx buf size must be >= permessage-deflate rx size */
        },
        {
                "lws-mirror-protocol",
                callback_lws_mirror,
                sizeof(struct per_session_data__lws_mirror),
                128, /* rx buf size must be >= permessage-deflate rx size */
        },
        {
                "lws-echogen",
                callback_lws_echogen,
                sizeof(struct per_session_data__echogen),
                128, /* rx buf size must be >= permessage-deflate rx size */
        },
        {
                "lws-status",
                callback_lws_status,
                sizeof(struct per_session_data__lws_status),
                128, /* rx buf size must be >= permessage-deflate rx size */
        },
        { NULL, NULL, 0, 0 } /* terminator */
};




static const struct lws_extension exts[] = {
        {
                "permessage-deflate",
                lws_extension_callback_pm_deflate,
                "permessage-deflate"
        },
        {
                "deflate-frame",
                lws_extension_callback_pm_deflate,
                "deflate_frame"
        },
        { NULL, NULL, NULL /* terminator */ }
};



static struct option options[] = {
        { "help",       no_argument,            NULL, 'h' },
        { "debug",      required_argument,      NULL, 'd' },
        { "port",       required_argument,      NULL, 'p' },
        { "ssl",        no_argument,            NULL, 's' },
        { "allow-non-ssl",      no_argument,    NULL, 'a' },
        { "interface",  required_argument,      NULL, 'i' },
        { "closetest",  no_argument,            NULL, 'c' },
        { "ssl-cert",  required_argument,       NULL, 'C' },
        { "ssl-key",  required_argument,        NULL, 'K' },
        { "ssl-ca",  required_argument,         NULL, 'A' },
#if defined(LWS_OPENSSL_SUPPORT)
        { "ssl-verify-client",  no_argument,            NULL, 'v' },
#if defined(LWS_HAVE_SSL_CTX_set1_param)
        { "ssl-crl",  required_argument,                NULL, 'R' },
#endif
#endif
        { "libev",  no_argument,                NULL, 'e' },
#ifndef LWS_NO_DAEMONIZE
        { "daemonize",  no_argument,            NULL, 'D' },
#endif
        { "resource_path", required_argument,   NULL, 'r' },
        { "pingpong-secs", required_argument,   NULL, 'P' },
        { NULL, 0, 0, 0 }
};


int main(int argc, char **argv)
{
  
        printf("can i get this to compile");
        struct lws_context_creation_info info;
        char interface_name[128] = "";
        unsigned int ms, oldms = 0;
        const char *iface = NULL;
        char cert_path[1024] = "";
        char key_path[1024] = "";
        char ca_path[1024] = "";
        int uid = -1, gid = -1;
        int use_ssl = 0;
        int pp_secs = 0;
        int opts = 0;
        int n = 0;
#ifndef _WIN32
/* LOG_PERROR is not POSIX standard, and may not be portable */
#ifdef __sun
        int syslog_options = LOG_PID;
#else        
        int syslog_options = LOG_PID | LOG_PERROR;
#endif
#endif
#ifndef LWS_NO_DAEMONIZE
        int daemonize = 0;
#endif

        /*
         * take care to zero down the info struct, he contains random garbaage
         * from the stack otherwise
         */
        memset(&info, 0, sizeof info);
        info.port = 7681;

        while (n >= 0) {
                n = getopt_long(argc, argv, "eci:hsap:d:Dr:C:K:A:R:vu:g:P:", options, NULL);
                if (n < 0)
                        continue;
                switch (n) {
                case 'e':
                        opts |= LWS_SERVER_OPTION_LIBEV;
                        break;
#ifndef LWS_NO_DAEMONIZE
                case 'D':
                        daemonize = 1;
                        #if !defined(_WIN32) && !defined(__sun)
                        syslog_options &= ~LOG_PERROR;
                        #endif
                        break;
#endif
                case 'u':
                        uid = atoi(optarg);
                        break;
                case 'g':
                        gid = atoi(optarg);
                        break;
                case 'd':
                        debug_level = atoi(optarg);
                        break;
                case 's':
                        use_ssl = 1;
                        break;
                case 'a':
                        opts |= LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT;
                        break;
                case 'p':
                        info.port = atoi(optarg);
                        break;
                case 'i':
                        strncpy(interface_name, optarg, sizeof interface_name);
                        interface_name[(sizeof interface_name) - 1] = '\0';
                        iface = interface_name;
                        break;
                case 'c':
                        close_testing = 1;
                        fprintf(stderr, " Close testing mode -- closes on "
                                           "client after 50 dumb increments"
                                           "and suppresses lws_mirror spam\n");
                        break;
                case 'r':
                        resource_path = optarg;
                        printf("Setting resource path to \"%s\"\n", resource_path);
                        break;
                case 'C':
                        strncpy(cert_path, optarg, sizeof(cert_path) - 1);
                        cert_path[sizeof(cert_path) - 1] = '\0';
                        break;
                case 'K':
                        strncpy(key_path, optarg, sizeof(key_path) - 1);
                        key_path[sizeof(key_path) - 1] = '\0';
                        break;
                case 'A':
                        strncpy(ca_path, optarg, sizeof(ca_path) - 1);
                        ca_path[sizeof(ca_path) - 1] = '\0';
                        break;
                case 'P':
                        pp_secs = atoi(optarg);
                        lwsl_notice("Setting pingpong interval to %d\n", pp_secs);
                        break;
#if defined(LWS_OPENSSL_SUPPORT)
                case 'v':
                        use_ssl = 1;
                        opts |= LWS_SERVER_OPTION_REQUIRE_VALID_OPENSSL_CLIENT_CERT;
                        break;
#if defined(LWS_USE_POLARSSL)
#else
#if defined(LWS_USE_MBEDTLS)
#else
#if defined(LWS_HAVE_SSL_CTX_set1_param)
                case 'R':
                        strncpy(crl_path, optarg, sizeof(crl_path) - 1);
                        crl_path[sizeof(crl_path) - 1] = '\0';
                        break;
#endif
#endif
#endif
#endif
                case 'h':
                        fprintf(stderr, "Usage: test-server "
                                        "[--port=<p>] [--ssl] "
                                        "[-d <log bitfield>] "
                                        "[--resource_path <path>]\n");
                        exit(1);
                }
        }

#if !defined(LWS_NO_DAEMONIZE) && !defined(WIN32)
        /*
         * normally lock path would be /var/lock/lwsts or similar, to
         * simplify getting started without having to take care about
         * permissions or running as root, set to /tmp/.lwsts-lock
         */
        if (daemonize && lws_daemonize("/tmp/.lwsts-lock")) {
                fprintf(stderr, "Failed to daemonize\n");
                return 10;
        }
#endif

        signal(SIGINT, sighandler);

#ifndef _WIN32
        /* we will only try to log things according to our debug_level */
        setlogmask(LOG_UPTO (LOG_DEBUG));
        openlog("lwsts", syslog_options, LOG_DAEMON);
#endif

        /* tell the library what debug level to emit and to send it to syslog */
        lws_set_log_level(debug_level, lwsl_emit_syslog);

        lwsl_notice("libwebsockets test server - license LGPL2.1+SLE\n");
        lwsl_notice("(C) Copyright 2010-2016 Andy Green <andy@warmcat.com>\n");

        printf("Using resource path \"%s\"\n", resource_path);
#ifdef EXTERNAL_POLL
        max_poll_elements = getdtablesize();
        pollfds = malloc(max_poll_elements * sizeof (struct lws_pollfd));
        fd_lookup = malloc(max_poll_elements * sizeof (int));
        if (pollfds == NULL || fd_lookup == NULL) {
                lwsl_err("Out of memory pollfds=%d\n", max_poll_elements);
                return -1;
        }
#endif

        info.iface = iface;
        info.protocols = protocols;
        info.ssl_cert_filepath = NULL;
        info.ssl_private_key_filepath = NULL;
        info.ws_ping_pong_interval = pp_secs;

        if (use_ssl) {
                if (strlen(resource_path) > sizeof(cert_path) - 32) {
                        lwsl_err("resource path too long\n");
                        return -1;
                }
                if (!cert_path[0])
                        sprintf(cert_path, "%s/libwebsockets-test-server.pem",
                                                                resource_path);
                if (strlen(resource_path) > sizeof(key_path) - 32) {
                        lwsl_err("resource path too long\n");
                        return -1;
                }
                if (!key_path[0])
                        sprintf(key_path, "%s/libwebsockets-test-server.key.pem",
                                                                resource_path);

                info.ssl_cert_filepath = cert_path;
                info.ssl_private_key_filepath = key_path;
                if (ca_path[0])
                        info.ssl_ca_filepath = ca_path;
        }
        info.gid = gid;
        info.uid = uid;
        info.max_http_header_pool = 16;
        info.options = opts | LWS_SERVER_OPTION_VALIDATE_UTF8;
        info.extensions = exts;
        info.timeout_secs = 5;
        info.ssl_cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:"
                               "ECDHE-RSA-AES256-GCM-SHA384:"
                               "DHE-RSA-AES256-GCM-SHA384:"
                               "ECDHE-RSA-AES256-SHA384:"
                               "HIGH:!aNULL:!eNULL:!EXPORT:"
                               "!DES:!MD5:!PSK:!RC4:!HMAC_SHA1:"
                               "!SHA1:!DHE-RSA-AES128-GCM-SHA256:"
                               "!DHE-RSA-AES128-SHA256:"
                               "!AES128-GCM-SHA256:"
                               "!AES128-SHA256:"
                               "!DHE-RSA-AES256-SHA256:"
                               "!AES256-GCM-SHA384:"
                               "!AES256-SHA256";

        if (use_ssl)
                /* redirect guys coming on http */
                info.options |= LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS;

        
        
        
        
        
        
        
        
        context = lws_create_context(&info);
        if (context == NULL) {
                lwsl_err("libwebsocket init failed\n");
                return -1;
        }
        
}
