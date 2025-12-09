*** Settings ***
Library    SerialLibrary
Library    String
Library    InfluxLib.py

*** Variables ***
${PORT}       /dev/ttyACM0
${BAUD}       115200
${TIMEOUT}    5

*** Test Cases ***
Stream Temp And Current To Influx
    Connect              ${PORT}    ${BAUD}
    Set Timeout          ${TIMEOUT}
    Reset Input Buffer
    Reset Output Buffer

    WHILE    True
        # --- 1) TEMPERATURE LINE (e.g. "23\r\n") ---
        ${t_line}=        Read Until    \n
        Log               Raw temperature line: ${t_line}

        ${t_clean}=       Strip String    ${t_line}
        ${temp}=          Convert To Number    ${t_clean}

        Should Be True    ${temp} >= -40 and ${temp} <= 125

        Log               Temperature: ${temp} °C
        Log To Console    >>> Temperature: ${temp} °C
        Write Temp To Influx    ${temp}

        # --- 2) CURRENT LINE (e.g. "I=-3\r\n") ---
        ${i_line}=        Read Until    \n
        Log               Raw current line: ${i_line}

        ${i_clean}=       Strip String    ${i_line}
        # Expect format "I=<value>"
        ${parts}=         Split String    ${i_clean}    =
        Length Should Be  ${parts}    2

        ${curr_str}=      Set Variable    ${parts[1]}
        ${current}=       Convert To Number    ${curr_str}

        Should Be True    ${current} >= -10 and ${current} <= 10

        Log               Current: ${current} A
        Log To Console    >>> Current: ${current} A
        Write Current To Influx    ${current}
    END

    Disconnect 
