typedef enum L3_event
{
    L3_event_msgRcvd = 2,
    L3_event_dataToSend = 4,
    L3_event_dataSendCnf = 5,
    L3_event_recfgSrcIdCnf = 6,

    // FSM event 추가
    L3_event_joinReq        = 7,
    L3_event_speakReq       = 8,
    L3_event_timeout        = 9,
    L3_event_queueExist     = 10,
    L3_event_queueEmpty     = 11,
    L3_event_adminRejoin    = 12,
    L3_event_tokenGrant     = 13,
    L3_event_tokenReturn    = 14,
    L3_event_userLeave      = 15

} L3_event_e;


void L3_event_setEventFlag(L3_event_e event);
void L3_event_clearEventFlag(L3_event_e event);
void L3_event_clearAllEventFlag(void);
int L3_event_checkEventFlag(L3_event_e event);