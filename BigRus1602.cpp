#include "BigRus1602.h"
#include "BigRus1602Font.h"

BigRus1602::BigRus1602(LiquidCrystal_I2C *lcd)
{
    _lcd = lcd;
}

void BigRus1602::begin(uint8_t initColumn, uint8_t initRow) // запишем свои символы (8 штук) в память экрана CGRAM (Character generator RAM)
{
    _lcd->begin(initColumn, initRow);
    uint8_t buffer[8];
    for (uint8_t k = 0; k < 8; k++)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            buffer[i] = grapheme[k][i];
        }
        _lcd->createChar(k, buffer);
    }
}

void BigRus1602::bufSize(uint8_t bufSize) // размер буффера с "чистыми" символами
{
    _bufSize = bufSize;
}

void BigRus1602::scrollText(bool scroll, uint16_t scrollSpeed) // скроллинг текста по вертикали и скорость
{
    _scrollText = scroll;
    _scrollSpeed = scrollSpeed;
}

void BigRus1602::doScrollText()
{
    for (uint8_t i = 0; i < _scrollIterration; i++) // _scrollIterration
    {
        _lcd->command(0x18); // LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT

        while ((millis() - _lastMillis) < _scrollSpeed)
        {
        }
        _lastMillis = millis();
    }
    _lcd->clear();
}

#if defined(BC_STRONG_FONT) || defined(BC_SMOOTH_FONT)

void BigRus1602::print(const char *inputData, uint8_t column)
{
    uint16_t lenDirtCharArray = strlen(inputData); // высчитываем количество "грязных" символов
    uint8_t clearCharArray[_bufSize];             // выделяем память для "чистых" символов
    uint8_t lenClearCharArray = 0;                // количество "чистых" символов
    uint8_t buff[BC_DEFAULT_BUF_SIZE];            // здесь храним часть от clearCharArray[], которую пользователь будет видеть на экране (max 40 символов в строке)

    for (uint16_t numbDirtSymbol = 0; numbDirtSymbol < lenDirtCharArray; numbDirtSymbol++) // перебор "грязных" символов
    {
        if ((uint8_t)inputData[numbDirtSymbol] == 32 ||                                    // пробел
            ((uint8_t)inputData[numbDirtSymbol] >= 144 && (uint8_t)inputData[numbDirtSymbol] <= 175) || // 144 - А, 175 - Я
            ((uint8_t)inputData[numbDirtSymbol] >= 48 && (uint8_t)inputData[numbDirtSymbol] <= 57) ||   // 48 - 0, 57 - 9
            (uint8_t)inputData[numbDirtSymbol] == 33 ||                                    // !
            ((uint8_t)inputData[numbDirtSymbol] >= 44 && (uint8_t)inputData[numbDirtSymbol] <= 46) ||   // 44 - , 45 - -, 46 - .
            (uint8_t)inputData[numbDirtSymbol] == 58 ||                                    // :
            (uint8_t)inputData[numbDirtSymbol] == 59 ||                                    // ;
            (uint8_t)inputData[numbDirtSymbol] == 63 ||                                    // ?
            (uint8_t)inputData[numbDirtSymbol] == 129)                                     // Ё
        {
            clearCharArray[lenClearCharArray++] = (uint8_t)inputData[numbDirtSymbol];
        }
    }

    for (uint8_t i = 0; i < lenClearCharArray; i++) // перебираем все чистые символы
    {
        _column = column;

        if ((lenClearCharArray - i) >= BC_DEFAULT_BUF_SIZE)
        {
            memcpy(buff, &(clearCharArray[i]), BC_DEFAULT_BUF_SIZE);
            this->write(buff, BC_DEFAULT_BUF_SIZE);
        }
        else
        {
            memcpy(buff, &(clearCharArray[i]), (lenClearCharArray - i));
            this->write(buff, (lenClearCharArray - i));
        }

        if (_scrollText == false)
            i = lenClearCharArray; // если скролл отключен, то устанавливаем счетчик на последний символ
        else                       // иначе скроллим
        {
            _scrollIterration -= column;
            this->doScrollText();
        }
    }
}

#endif

void BigRus1602::print(uint32_t inputData, uint8_t column)
{
    _column = column;

    if (inputData == 0) // если пришел нуль
    {
        this->write((uint8_t *)"\x1F", 1);
    }
    else // если пришло не нулевое значение
    {
        uint8_t numLength = 10;  // количество цифр в пришедшем числе (максимум 10)
        uint8_t buff[numLength]; // буфер цифр

        // разбиваем число на цифры и вычисляем их реальное количество
        numLength = 0; // сбросим numLength чтобы пересчитать реальное количество цифр в числе
        while (inputData != 0)
        {
            buff[numLength++] = inputData % 10;
            inputData /= 10;
        }

        uint8_t bytes[numLength];

        // инвертируем буфер
        for (uint8_t i = 0; i < numLength; i++)
        {
            bytes[i] = buff[numLength - (i + 1)];
        }

        this->write(bytes, numLength);
    }
}

void BigRus1602::write(const uint8_t *inputData, uint8_t sizeArr)
{
    for (uint8_t i = 0; i < sizeArr; i++)
    {
        _row = 0;
        switch (inputData[i])
        {
#if defined(BC_SMOOTH_FONT)
        case 0x20: // пробел
            _lcd->setCursor(_column, _row);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write(' ');
            _column -= 2;
            break;

        case 0x90: // А
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)3);
            _lcd->write(' ');
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0x91: // Б
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)5);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)6);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0x92: // В
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)6);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            break;

                case 0x94: // Д
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0x81: // Ё
        case 0x95: // Е
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)5);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)6);
            _lcd->write((uint8_t)6);
            _lcd->write(' ');
            break;

        case 0x96: // Ж
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)3);
            _lcd->write(' ');
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _column += 2;
            break;

        case 0x97: // З
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)6);
            _lcd->write((uint8_t)6);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0x98: // И
        case 0x99: // Й
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)3);
            _lcd->write((uint8_t)4);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0x9A: // К
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)255);
            _lcd->write(' ');
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            break;

        case 0x9B: // Л
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _lcd->write((uint8_t)255);
            _lcd->write(' ');
            break;

        case 0x9C: // М
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)4);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)3);
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _column += 2;
            break;

        case 0x9D: // Н
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)3);
            _lcd->write(' ');
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0x9E: // О
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)3);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0x9F: // П
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)255);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)255);
            _lcd->write(' ');
            _lcd->write((uint8_t)255);
            _lcd->write(' ');
            break;

        case 0xA0: // Р
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)255);
            _lcd->write(' ');
            break;

        case 0xA1: // С
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)7);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)3);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)10);
            _lcd->write(' ');
            break;

        case 0xA2: // Т
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)7);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write(' ');
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0xA3: // У
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

               case 0xA4: // Ф
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write(' ');
            _lcd->write((uint8_t)255);
            _lcd->write(' ');
            _lcd->write(' ');
            _column += 2;
            break;

        case 0xA5: // Х
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)3);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            break;

        case 0xA6: // Ц
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)3);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)10);
            _lcd->write(' ');
            _column += 1;
            break;

        case 0xA7: // Ч
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0xA8: // Ш
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)3);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _column += 2;
            break;

        case 0xA9: // Щ
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)3);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)10);
            _lcd->write(' ');
            _column += 3;
            break;

        case 0xAA: // Ъ
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write(' ');
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)6);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _column += 1;
            break;

        case 0xAB: // Ы
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)6);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _column += 2;
            break;

        
        case 0xAC: // Ь
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)255);
            _lcd->write((uint8_t)6);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0xAD: // Э
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)6);
            _lcd->write((uint8_t)6);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            break;

        case 0xAE: // Ю
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)2);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _lcd->write((uint8_t)3);
            _lcd->write((uint8_t)10);
            _lcd->write((uint8_t)4);
            _lcd->write(' ');
            _column += 2;
            break;

        case 0xAF: // Я
            _lcd->setCursor(_column, _row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)5);
            _lcd->write((uint8_t)255);
            _lcd->write(' ');
            _lcd->setCursor(_column, ++_row);
            _lcd->write((uint8_t)1);
            _lcd->write((uint8_t)7);
            _lcd->write((uint8_t)255);
            _lcd->write(' ');
            break;

    default: // если пришел неизвестный символ
        break;
    }

    _column += 4;

    if (i == 0)
        _scrollIterration = _column;
    }
}

#endif // Закрытие условной компиляции
