# PipeSum

(ENG)

Operating system: Linux

The program consists of three producer-consumer processes that use the mechanism of communication links (pipes). The program allows for processing numerical data, which is transmitted between processes in text form. Process 1 reads lines consisting of numbers separated by "+" and passes them to process 2. Process 2 verifies whether each expression consists of numbers and "+", and then passes correct data to process 3. Process 3 receives the data, calculates the sum, and outputs the result in separate lines.

Example of valid input data:
* 3+5
* 23+9

To stop/resume/terminate the program, send the appropriate signals to any process from a separate terminal:
* SIGUSR1
* SIGUSR2
* SIGINT

<br><br>
(PL)

System operacynjy: Linux 

Program składający się z trzech procesów typu producent-konsument wykorzystujących mechanizm łączy komunikacyjnych (pipes).
Program umożliwia przetwarzanie danych liczbowych, które są przekazywane między procesami w formie tekstowej. Proces 1 odczytuje wiersze składające się z liczb oddzielonych znakiem "+" i przekazuje do procesu 2. Proces 2 weryfikuje, czy każde wyrażenie składa się z liczb oraz znaków "+", a następnie przekazuje poprawne dane do procesu 3. Proces 3 otrzymuje dane, wylicza sumę i wyprowadza wynik w osobnych wierszach.

Przykład poprwanych danych wejściowych:
* 3+5
* 23+9

Aby zatrzymać/wznowić/zakończyć działanie programu należy wysłać z osobnego terminala odpowiednio sygnały do dowolnego procesu:
* SIGUSR1
* SIGUSR2
* SIGINT
