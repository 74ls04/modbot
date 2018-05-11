#include "Arduino.h"
#include <Wire.h>
#include <Stackmodio.h>


// Class Variables //////////////////////////////////////////////////

// Data Registers
byte			StackModIO::motors_reg[MAX_REGISTER_SIZE] = { 0 };
byte			StackModIO::servo_reg[MAX_REGISTER_SIZE];
byte			StackModIO::ultra_reg[MAX_REGISTER_SIZE];
byte			StackModIO::ir_reg[MAX_REGISTER_SIZE];
char            StackModIO::packet_buffer[MAX_BYTES] = { 0 };
char            StackModIO::current_command[MAX_BYTES] = { 0 };
uint8_t			StackModIO::motors[MAX_BYTES] = { 0 };

bool			StackModIO::NEWPACKET = false;
bool			StackModIO::DEBUGGING = true;
bool			StackModIO::RECEIVING = false;
uint8_t         StackModIO::i2c_address = 0;

// Structs and enums //////////////////////////////////////////////////

// Available commands
enum Cmd_ids
{
    ARM_MOTORS,
    MOTOR,
    SERVO,
    ULTRASONIC,
    IR,
    DIGITAL,
    ANALOG,
    NUM_COMMANDS
};

struct Commands
{
    char *cmd;              // cmd
    unsigned char data_len; // length
};

static Commands commands[] =
    { //Cmd   //Length //Register address
        "ARM", 1,
        "MTR", 8,
        "SRV", 8,
        "ULT", 8,
        "IRS", 8,
        "DGT", 1,
        "ANL", 1,
        0
    };


struct MOTORS {
	int motor_1;
	int motor_2;
	int motor_3;
	int motor_4;
	int motor_5;
	int motor_6;
	int motor_7;
	int motor_8;
};

// typdefs that allor for convenient access to our data registers
typedef struct
{
    int motor_1;
    int motor_2;
    int motor_3;
    int motor_4;
    int motor_5;
    int motor_6;
    int motor_7;
    int motor_8;
} motors_t;

typedef struct
{
    unsigned short val_1 : 16;
    unsigned short val_2 : 16;
    unsigned short val_3 : 16;
    unsigned short val_4 : 16;
    unsigned short val_5 : 16;
    unsigned short val_6 : 16;
    unsigned short val_7 : 16;
    unsigned short val_8 : 16;
} large_pck_t;



byte StackModIO::receiveData(byte inByte)
{
    static byte index = 0;
    char startMarker = '{';
    char endMarker = '}';

	if (inByte == '\0')
	{
		memset(packet_buffer, 0, MAX_BYTES); // Empty char buffer
		RECEIVING = false;
	}

    if (!NEWPACKET)
    {

        if (RECEIVING && inByte != startMarker)
        {
            if (inByte != endMarker)
            {
                packet_buffer[index] = inByte;
                index++;
                if (index >= MAX_BYTES)
                {
                    index = MAX_BYTES - 1;
                }
            }
            else
            {
                packet_buffer[index] = '\0'; // terminate the string
				memcpy(current_command, packet_buffer, sizeof(packet_buffer));
				memset(packet_buffer, 0, MAX_BYTES); // Empty char buffer
                RECEIVING = false;
                index = 0;
                NEWPACKET = true;
            }
        }
        else if (inByte == startMarker)
        {
            RECEIVING = true;
            memset(packet_buffer, 0, MAX_BYTES); // Empty char buffer
        }
    }

	if (NEWPACKET) processPacket();
	return inByte;
}



// Public Methods ///////////////////////////////////////////////////////////////

void StackModIO::begin(uint8_t slave_address)
{
    i2c_address = slave_address;
    Wire.begin(slave_address); 
	// Wire.onReceive(receiveEvent); 
	// Wire.onRequest(requestEvent);
}

void StackModIO::setMotorSpeed(uint8_t motor_number, byte speed)
{
	if (motor_number > 0 && motor_number < MAX_REGISTER_SIZE + 1)
	{
		motors_reg[motor_number - 1] = speed;
	}
}

void StackModIO::setMotorRange(int min, int max)
{
}

int StackModIO::getMotorSpeed(uint8_t motor_number)
{
	if (motor_number > 0 && motor_number < MAX_REGISTER_SIZE + 1)
	{
		return motors_reg[motor_number - 1];
	}
	else {
		return 0;
	}
}

// Private Methods //////////////////////////////////////////////////////////////

/*
void get_speeds()
{
    motors_t *motor_speeds = (motors_t *)motors_reg;

    Serial.println((int)motor_speeds->motor_1_speed);
    Serial.println((int)motor_speeds->motor_5_speed);
}

 int StackModIO::getMotorSpeed(int motor_number) 
 {
    small_pckt_t *motor_speeds = (small_pckt_t *) motors_reg;
    Serial.println((int)motor_speeds->val_1);
 }   

 */


void StackModIO::processPacket()
{
	//serial.println("HB");
	char data[MAX_REGISTER_SIZE + 1];

	// Clear the packet status
	NEWPACKET = false;

	//serial.println(packet_buffer);

	// Validate minimal packet size
	unsigned char packet_size = strlen(current_command);

	if (packet_size < 5)
	{
		// serial.println("Invalid packet size ");
		return;
	}

	// Get target address of packet
	uint8_t address = current_command[0];


	// Check if command or query
	uint8_t action = current_command[1];

	// Identify which command was sent
	int cmd;
	for (cmd = 0; commands[cmd].cmd; cmd++)
	{
		if (!strncmp(&current_command[2], commands[cmd].cmd, strlen(commands[cmd].cmd)))
			break;
	}

	// Address must match our address

	if (address != i2c_address) {
		// if (DEBUGGING) Serial.println("INVALID ADDRESS");
		return;
	}


	switch (cmd)
	{
	case MOTOR:

		// if (DEBUGGING)
			// serial.println("This is a motor command");

		// Copy packet to temp array and verify length
		snprintf(data, commands[cmd].data_len + 1, "%s", &current_command[5]);

		if (strlen(data) == commands[cmd].data_len)
		{
			memcpy(motors_reg, data, sizeof(data));

			// Clean up all the temp data
			memset(data, 0, sizeof(data));
			// memset(packet_buffer, 0, MAX_BYTES); // Empty char buffer
		}
		else
		{
			//if (DEBUGGING)
				//serial.println("Invalid motor string length");
		}

		//Serial.println(motors_reg);
		break;
	case ULTRASONIC:
		//if (DEBUGGING)
			//serial.println("This is an ultrasonic command");
		break;
	default:
		//serial.println("Command not found");
		break;
	}

}
