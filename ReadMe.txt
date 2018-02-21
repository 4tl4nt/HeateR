Шаг 1. Для начало работы, необходимо импортировать библиотеки HeateR, в среду Arduino. 
Для этого необходимо запустить Create_ZIP.bat 
Этот скрипт создает ZIP архив и добавляет в него содержимое папок CLI, communications и Heater
Созданную ZIP библиотеку необходимо импортировать в среду как показано в инструкции: https://www.arduino.cc/en/Guide/Libraries

Шаг 2. Для работы библиотеки HeateR нужно добавить несколько внешних библиотек
Библиотека для работы с датчиками температуры Dallas https://github.com/milesburton/Arduino-Temperature-Control-Library
Библиотека для работы расширителя портов hhttps://github.com/skywodd/pcf8574_arduino_library/tree/master/PCF8574
Библиотека для работы OneWire https://github.com/ntruchsess/arduino-OneWire

Шаг 3. Открыть файл MainController\MainController.ino с помощью arduino ide, выбрать плату Mega 2560 и нажать кнопку загрузка

