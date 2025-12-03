*** Settings ***
Library    SerialLibrary
Library    String

*** Variables ***
${PORT}       /dev/ttyACM0
${BAUD}       115200
${TIMEOUT}    5

*** Test Cases ***
Read Temperature Over UART
    Connect              ${PORT}    ${BAUD}
    Set Timeout          ${TIMEOUT}
    Reset Input Buffer
    Reset Output Buffer

    Write                temp

    ${data}=             Read Until    \n
    Log                  Got from MCU raw: ${data}

    ${clean}=            Replace String    ${data}    \r\n    ${EMPTY}
    ${clean}=            Replace String    ${clean}   \n      ${EMPTY}
    ${clean}=            Replace String    ${clean}   \r      ${EMPTY}

    ${temp}=             Convert To Number    ${clean}

    Should Be True       ${temp} >= -40 and ${temp} <= 125

    Log                  Temperature: ${temp} °C

    Log To Console       >>> Temperature: ${temp} °C

    Disconnect
