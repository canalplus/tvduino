/*
 * Rest API Server for Arduino
 * 
 * Filipe C - fcaldas@canal-plus.fr
 */

#define ROUTESIZE        20
#define MAX_REQUEST_SIZE 80

enum REQUEST_TYPE {
  GET,
  POST
};

class __request{
public:
  //route name is written as a char array
  //String datatype was causing a memory leak
  char routename[ROUTESIZE];
  REQUEST_TYPE rtype;
};


class __route{

public:
  char routename[ROUTESIZE];
  REQUEST_TYPE rtype;
  void (*callback)(EthernetClient *client, char args[]);

  __route(char rname[], REQUEST_TYPE req, 
          void (*callbackf)(EthernetClient *client, char args[]) ){
    callback = callbackf;
    strcpy(routename,rname);
    rtype = req;
  }
  __route(){}
  
};

