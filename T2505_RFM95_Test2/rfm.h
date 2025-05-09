#ifndef __RFM_H__
#define __RFM_H__


typedef enum
{
    DEFAULT_ADDRESS = 0,
    CLIENT_ADDRESS  = 1,
    SERVER_ADDRESS  = 2
} addr_type_et;

typedef enum
{
    NODE_ROLE_UNDEFINED = 0,
    NODE_ROLE_CLIENT,
    NODE_ROLE_SERVER,
    NODE_ROLE_RELIABLE_CLIENT,
    NODE_ROLE_RELIABLE_SERVER

} node_role_et;

typedef struct 
{
    uint8_t      node_addr;
    node_role_et node_role;
    uint16_t     client_cntr;
    uint16_t     server_cntr;
} rfm_ctrl_st;

void rfm_initialize(node_role_et node_role);

void rfm_loop(void);



#endif