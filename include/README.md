
This directory should include a "credentials.h" file containing the following content.

    // assign network name (ssid) and password (password) 
    const char* ssid     = "ssidname";
    const char* password = "password";  // must be min 8 char or blank for open network

    // assign IP address
    IPAddress local_IP(10,0,0,200);     // url of access point (http://10.0.0.200)
    IPAddress gateway(10,0,0,1);
    IPAddress subnet(255,255,255,0);

Adjust the ssid and password to suit your site.
