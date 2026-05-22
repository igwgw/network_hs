#include "L3_FSMevent.h"
#include "L3_msg.h"
#include "L3_timer.h"
#include "L3_LLinterface.h"
#include "protocol_parameters.h"
#include "mbed.h"
#include <stdio.h>
#include <string.h>



//FSM state -------------------------------------------------
#define L3STATE_IDLE                0
#define L3STATE_JOIN                1
#define L3STATE_TX                  2
#define L3STATE_TOKEN               3
#define L3STATE_TOKEN_WAIT          4
#define L3STATE_ADMIN               5


//condition variables ---------------------------------------
static uint8_t C1_token_using = 0;       // C1: 토큰 사용 중
static uint8_t C2_admin_exist = 0;       // C2: 관리자 존재
static uint8_t C3_admin_id_match = 0;    // C3: 관리자 ID 일치
static uint8_t C4_id_duplicate = 0;      // C4: ID 중복


//state variables
static uint8_t main_state = L3STATE_IDLE;
static uint8_t prev_state = main_state;


//SDU (input)
static uint8_t originalWord[1030];
static uint8_t wordLen=0;
static uint8_t sdu[1030];


//serial port interface
static Serial pc(USBTX, USBRX);
static uint8_t myDestId;
static char myNickname[20];

// 오늘 추가
static uint8_t userRegistered = 0;


//application event handler : generating SDU from keyboard input
static void L3service_processInputWord(void)
{
    char c = pc.getc();

    if (!L3_event_checkEventFlag(L3_event_dataToSend))
    {
        if (c == '\n' || c == '\r')
        {
            // 아무 글자도 안 치고 엔터만 누르면 무시
            if (wordLen == 0)
            {
                return;
            }

            originalWord[wordLen] = '\0';

            // join 명령어 없이 입력 완료되면 바로 dataToSend 이벤트 발생
            L3_event_setEventFlag(L3_event_dataToSend);

            debug_if(DBGMSG_L3,
                     "word is ready! ::: %s\n",
                     originalWord);
        }
        else
        {
            originalWord[wordLen++] = c;

            if (wordLen >= L3_MAXDATASIZE - 1)
            {
                originalWord[wordLen] = '\0';

                L3_event_setEventFlag(L3_event_dataToSend);

                pc.printf("\nmax reached! word forced to be ready :::: %s\n",
                          originalWord);
            }
        }
    }
}

void L3_initFSM(uint8_t destId)
{
    myDestId = destId;
    
    pc.attach(&L3service_processInputWord, Serial::RxIrq);

    pc.printf("Input Nickname : ");
}


void L3_FSMrun(void)
{  
    if (prev_state != main_state)
    {
        debug_if(DBGMSG_L3,
                 "[L3] State transition from %i to %i\n",
                 prev_state,
                 main_state);

        prev_state = main_state;
    }

    switch (main_state)
    {
        // -------------------------------------------------
        // state 0. IDLE
        // -------------------------------------------------
        case L3STATE_IDLE:

            // 처음 입력한 값은 닉네임으로 등록
            if (L3_event_checkEventFlag(L3_event_dataToSend) && userRegistered == 0)
            {
                strcpy(myNickname, (char*)originalWord);

                userRegistered = 1;

                pc.printf("\n===== JOIN STATE =====\n");
                pc.printf("User Registered.\n");
                pc.printf("Nickname : %s\n", myNickname);
                pc.printf("Destination Node : %d\n", myDestId);

                sprintf((char*)sdu, "%s entered the chat.", myNickname);

                pc.printf("MY MSG : %s\n", sdu);

                L3_LLI_dataReqFunc(
                    sdu,
                    strlen((char*)sdu) + 1,
                    myDestId
                );

                wordLen = 0;
                L3_event_clearEventFlag(L3_event_dataToSend);

                pc.printf("Give a word to send : ");

                main_state = L3STATE_JOIN;
            }

            // 닉네임 등록 전/후 수신 메시지 출력
            else if (L3_event_checkEventFlag(L3_event_msgRcvd))
            {
                uint8_t* dataPtr = L3_LLI_getMsgPtr();
                uint8_t size = L3_LLI_getSize();

                pc.printf("\nRCVD MSG : %s (length:%i)\n", dataPtr, size);

                L3_event_clearEventFlag(L3_event_msgRcvd);

                if (userRegistered == 0)
                {
                    pc.printf("Input Nickname : ");
                }
                else
                {
                    pc.printf("Give a word to send : ");
                }
            }

            break;


        // -------------------------------------------------
        // state 1. JOIN
        // -------------------------------------------------
        case L3STATE_JOIN:

            // 등록 이후에는 일반 채팅 메시지 전송
            if (L3_event_checkEventFlag(L3_event_dataToSend))
            {
                sprintf((char*)sdu, "[%s] %s", myNickname, originalWord);

                pc.printf("\nMY MSG : %s\n", sdu);

                L3_LLI_dataReqFunc(
                    sdu,
                    strlen((char*)sdu) + 1,
                    myDestId
                );

                wordLen = 0;

                pc.printf("Give a word to send : ");

                L3_event_clearEventFlag(L3_event_dataToSend);
            }

            // 다른 사람이 보낸 입장/채팅 메시지 수신
            else if (L3_event_checkEventFlag(L3_event_msgRcvd))
            {
                uint8_t* dataPtr = L3_LLI_getMsgPtr();
                uint8_t size = L3_LLI_getSize();

                pc.printf("\nRCVD MSG : %s (length:%i)\n", dataPtr, size);

                pc.printf("Give a word to send : ");

                L3_event_clearEventFlag(L3_event_msgRcvd);
            }

            break;


        /*
        // -------------------------------------------------
        // 오늘은 사용하지 않는 state
        // -------------------------------------------------

        case L3STATE_TX:
            break;

        case L3STATE_TOKEN:
            break;

        case L3STATE_TOKEN_WAIT:
            break;

        case L3STATE_ADMIN:
            break;
        */

        default:
            main_state = L3STATE_IDLE;
            break;
    }
}