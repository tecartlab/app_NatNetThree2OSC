#include <iostream>
#include <asio.hpp>
#include <conio.h>
#include <ctime>
#include <string>
#include <asio.hpp>


class SimpleSerial
{
public:
    /**
    * Constructor.
    * \param port device name, example "/dev/ttyUSB0" or "COM4"
    * \param baud_rate communication speed, example 9600 or 115200
    * \throws boost::system::system_error if cannot open the
    * serial device
    */
    SimpleSerial( std::string port, unsigned int baud_rate )
        : io(), serial( io, port )
    {
        serial.set_option( asio::serial_port_base::baud_rate( baud_rate ) );
    }

    /**
    * Write a string to the serial device.
    * \param s string to write
    * \throws boost::system::system_error on failure
    */
    void writeString( std::string s )
    {
        asio::write( serial, asio::buffer( s.c_str(), s.size() ) );
    }

    /**
    * Blocks until a line is received from the serial device.
    * Eventual '\n' or '\r\n' characters at the end of the string are removed.
    * \return a string containing the received line
    * \throws boost::system::system_error on failure
    */
    std::string readLine()
    {
        //Reading data char by char, code is optimized for simplicity, not speed
        using namespace asio;
        char c;
        std::string result;
        while ( true )
        {
            asio::read( serial, asio::buffer( &c, 1 ) );
            switch ( c )
            {
            case '\r':
                return result;
            case '\n':
                return result;
            default:
                result += c;
            }
        }
    }

private:
    asio::io_service io;
    asio::serial_port serial;
};