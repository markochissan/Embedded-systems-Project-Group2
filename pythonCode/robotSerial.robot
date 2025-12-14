*** Settings ***
Library    SerialLibrary
Library    String

*** Variables ***
${PORT}       /dev/ttyACM0
${BAUD}       115200
${TIMEOUT}    5

*** Test Cases ***
Read Temperature Over UART
    [Documentation]    Reads temperature and verifies valid range
    Connect              ${PORT}    ${BAUD}
    Set Timeout          ${TIMEOUT}
    Reset Input Buffer
    Reset Output Buffer

    Write                temp
    ${data}=             Read Until    \n
    Log                  Got from MCU raw: ${data}

    ${clean}=            Clean UART String    ${data}
    ${temp}=             Convert To Number    ${clean}

    Should Be True       ${temp} >= -40 and ${temp} <= 125

    Log                  Temperature: ${temp} Â°C
    Log To Console       >>> Temperature: ${temp} Â°C

    Disconnect


Test MOSFET States Over UART
    [Documentation]    Verifies all MOSFET states via UART feedback
    Connect              ${PORT}    ${BAUD}
    Set Timeout          ${TIMEOUT}
    Reset Input Buffer
    Reset Output Buffer

    Verify MOSFET State    MfOp    Mosfet fully open
    Verify MOSFET State    MfCl    Mosfet fully closed
    Verify MOSFET State    MfCh    Mosfet charge
    Verify MOSFET State    MfDc    Mosfet discharge

    Disconnect


*** Keywords ***
Clean UART String
    [Arguments]    ${data}
    ${clean}=      Replace String    ${data}    \r\n    ${EMPTY}
    ${clean}=      Replace String    ${clean}   \n      ${EMPTY}
    ${clean}=      Replace String    ${clean}   \r      ${EMPTY}
    [Return]       ${clean}

Verify MOSFET State
    [Arguments]    ${command}    ${expected}

    Write          ${command}
    ${resp}=       Read Until    \n
    Log            MOSFET raw response: ${resp}

    ${clean}=      Clean UART String    ${resp}

    Should Contain    ${clean}    ${expected}

    Log To Console    >>> MOSFET OK: ${clean}
