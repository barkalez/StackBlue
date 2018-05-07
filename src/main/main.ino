/*
FECHA: 17/06/2017
AUTOR: Barkalez
*/
 
/****************************************************************
-------------------------------INFO------------------------------
****************************************************************/
/*
  Repository:
        https://github.com/barkalez/StackBlueV5
  Motor used:
      NEMA17
        Datasheet: https://cdn-shop.adafruit.com/product-files/324/C140-A+datasheet.jpg
        Declaration of Conformity
        RoHS
        200 steps per revolution, 1.8 degrees
        Coil #1: Red & Yellow wire pair. Coil #2 Green & Brown/Gray wire pair.
        Bipolar stepper, requires 2 full H-bridges!
        4-wire, 8 inch leads
        42mm/1.65" square body
        31mm/1.22" square mounting holes, 3mm metric screws (M3)
        5mm diameter drive shaft, 24mm long, with a machined flat
        12V rated voltage (you can drive it at a lower voltage, but the torque will drop) at 350mA max current
        28 oz*in, 20 N*cm, 2 Kg*cm holding torque per phase
        35 ohms per winding
  Driver used:
      DRV8825
        Modelo              A4988
              DRV8825
        Color           Verde o Rojo        Morado
        Intensidad máxima       2A            2.5A
        Tensión máxima          35V           45A
        Microsteps            16            32
 
         https://www.luisllamas.es/motores-paso-paso-arduino-driver-a4988-drv8825/
*/
/****************************************************************
------------------------Urls de interest-------------------------
****************************************************************/
/*
    1-.
 
    https://www.luisllamas.es/motores-paso-paso-arduino-driver-a4988-drv8825/
 
  2-.
    https://github.com/barkalez/StackBlueV5
 
*/
/****************************************************************
------------------------Librarys-------------------------------
****************************************************************/
 
#include <SoftwareSerial.h>        //Incluye library SoftwareSerial.h (Conexión Bluetooth)
#include <string.h>                //Incluye library string.h
#include <EEPROM.h>
#include <math.h>
 
/****************************************************************
------------------------Constantes-------------------------------
****************************************************************/
 
 
#define TK_POSICIONSTART       0
#define TK_POSICIONEND         1
#define TK_NUMPHOTOS           2
#define TK_FACTORACELERATION   3
#define TK_TIMEBEFORESHOOT     4
#define TK_TIMEAFTERSHOOT      5
#define TK_SPEEDMOTOR          6
#define TK_DIRMOTOR            7
#define TK_NUMSTEP             8
#define TK_GOPOSITION          9
#define TK_SAVEPSOTION         10
#define TK_COMANDO             11
 
#define NUMTOKENS              12
 
 
 
 
 
 
 
#define LEN_CADENA              1024
 
#define CMD_MOVERMOTOR          0x00000001    //   0000 0000 0000 0000 0000 0000 0000 0001
#define CMD_STOPMOTOR           0x00000002    //   0000 0000 0000 0000 0000 0000 0000 0010
#define CMD_SAVEPOSITION        0x00000004    //   0000 0000 0000 0000 0000 0000 0000 0100
#define CMD_STACKED             0x00000008    //   0000 0000 0000 0000 0000 0000 0000 1000
#define CMD_GO_POSITION         0x00000010    //   0000 0000 0000 0000 0000 0000 0001 0000
#define CMD_SET_CONFIG          0x00000020    //   0000 0000 0000 0000 0000 0000 0010 0000
#define CMD_GO_HOME             0x00000040    //   0000 0000 0000 0000 0000 0000 0100 0000
                              /*0x00000080
                                0x00000100
                                0x00000200
                                0x00000400
                                0x00000800
                                0x00001000
                                0x00002000
                                0x00004000
                                0x00008000
                                0x00010000
                                0x00020000
                                0x00040000
                                0x00080000
                                0x00100000
                                0x00200000
                                0x00400000
                                0x00800000*/
 
 
 
#define TOKEN                  '\n'
#define END_OF_MSSG            'T'
 
#define MICROSTEPS_SIXTEENTH        16
#define MICROSTEPS_EIGHTH           8
#define MICROSTEPS_QUARTER          4
#define MICROSTEPS_HALF             2
#define MICROSTEPS_FULL             1
 
#define EEPROM_ADDR_POSITION   0
#define EEPROM_ADDR_VERIFY     (EEPROM_ADDR_POSITION + sizeof(position))
#define EEPROM_ADDR_BACKLASS   (100)  //(EEPROM_ADDR_VERIFY + sizeof(verify_save))
#define EEPROM_ADDR_TYPEMOTOR  (200)
#define EEPROM_ADDR_MICROSTEP  (300)
#define EEPROM_ADDR_SCREWSTEP  (400)
#define EEPROM_VERIFY_FAIL     0x00
#define EEPROM_VERIFY_SUCCESS  0xff
 
#define PIN_KEY                2
#define PIN_STEP               3
#define PIN_MS0                4
#define PIN_SLEEP              5
#define PIN_RESET              6
#define PIN_MS1                7
#define PIN_MS2                8
#define PIN_ENABLEDRIV         9
#define PIN_TX                 10
#define PIN_RX                 11
#define PIN_DIR                12
#define PIN_ENDSTOP            13
 
#define MAX_STRING_LEN         20
 
#define DEFAULT_BACKLASS           5  //En micras
#define DEFAULT_TYPEMOTOR          200
#define DEFAULT_MICROSTEP          MICROSTEPS_FULL
#define DEFAULT_SCREWSTEP          3
#define DEFAULT_ACELERATION        1
#define DEFAULT_SPEED_MAX          100
#define STEP_UNIT                  1
#define DEFAULT_SPEED              300
#define DIR_POS_STEP               1
#define DIR_NEG_STEP               0
#define SpeedMin_Default           600
 
 
/****************************************************************
------------------------Global Variables-------------------------------
****************************************************************/
 
long            dato[NUMTOKENS];
char            CadenaAscii[LEN_CADENA];
unsigned long   position;
unsigned char   verify_save;
 
 
/****************************************************************
------------------------Bluetooth-------------------------------
****************************************************************/
 
SoftwareSerial StackBlue(PIN_TX, PIN_RX);   //Se crea conexión serie StackBlue(Rx,Tx);
 
/****************************************************************
------------------------Void Setup-------------------------------
****************************************************************/
 
void setup()
{
  pinMode(PIN_KEY,        OUTPUT);
  digitalWrite(PIN_KEY, HIGH);
  pinMode(PIN_STEP,       OUTPUT);
  pinMode(PIN_MS0,        OUTPUT);
  pinMode(PIN_SLEEP,      OUTPUT);
  pinMode(PIN_RESET,      OUTPUT);
  pinMode(PIN_MS1,        OUTPUT);
  pinMode(PIN_MS2,        OUTPUT);
  pinMode(PIN_ENABLEDRIV, OUTPUT);
  pinMode(PIN_DIR,        OUTPUT);
  //pinMode(PIN_ENDSTOP,    INPUT);
  pinMode(13, OUTPUT);
 
  StackBlue.begin(9600);            //Se configura velocidad de conexión de StackBlue
  Serial.begin(9600);              //Se configura velocidad de conexión de Arduino-PC
 
  memset(dato, 0, NUMTOKENS * sizeof(int)); //Limpia la memoria SRAM que ocupa la array datos de datos anteriores.
  memset(CadenaAscii, 0, LEN_CADENA * sizeof(char));
 
  //Lee la posición de la última vez que se guardó en la EEPROM
  //Lee la posición de la dirección EEPROM_ADDR_POSITION +1 y la desplaza 8 Bits a la izquierda y hace con ella un OR con lo que hay en la dirección EEPROM_ADDR_POSITION
  position    = ((unsigned long)(EEPROM.read(EEPROM_ADDR_POSITION + 3)) << 24)
              | ((unsigned long)(EEPROM.read(EEPROM_ADDR_POSITION + 2)) << 16)
              | ((unsigned long)(EEPROM.read(EEPROM_ADDR_POSITION + 1)) << 8)
              |  (unsigned long)EEPROM.read(EEPROM_ADDR_POSITION);
 
 
 
 
  // Grabamos en EEPROM la condicion de fallo de grabáción de posición
  EEPROM.write(EEPROM_ADDR_VERIFY, EEPROM_VERIFY_FAIL);
 
  //getConfig();
 
 
 
  //Establecemos la resolución del Motor
  //stepperResolution(Get_MicroStep);
 
  //Lee en la EEPROM si se grabó correctamente la posición de la plataforma en el último apagado
  /*verify_save = EEPROM.read(EEPROM_ADDR_VERIFY);
  //En el caso que no se grabó la position en el último apagado se resetea yendo al inicio (EndStop) y establecer a 0 la posición de la plataforma mediante la función GoHome()
  if(verify_save == EEPROM_VERIFY_FAIL)
  {
    // ERROR!! NO SE HA GRABADO BIEN
    goHome();
  }*/
}
 
/****************************************************************
------------------------Void loop-------------------------------
****************************************************************/
 
void loop()
{
    static int i = 0;
    int rd_cnt;
 
    //-- Leemos el numero de datos disponibles
    if((rd_cnt = StackBlue.available()) > 0)
    {
Serial.print("Num. Bytes: ");
Serial.println(rd_cnt);
        //-- Guardamos en CadenaAscii los datos disponibles
        for(int j = 0; j < rd_cnt; ++j)
        {
            CadenaAscii[i + j] = StackBlue.read();
            //-- Si hemos llegado al TOKEN END_OF_MSSG, procesamos
            //   la cadena y continuamos rellenando datos del bluetooth
            if(CadenaAscii[i + j] == END_OF_MSSG)
            {
Serial.println("Se lee fin de cadena, procesamos la misma");
                char* buff = CadenaAscii;
                char aux[16];
 
                for(int k = 0; k < NUMTOKENS; ++k)
                {
Serial.print("Token "); Serial.print(k); Serial.print(": \"");
                    sscanf(buff, "%s\n", aux);
                    dato[k] = atoi(aux);
Serial.print(aux); Serial.print("\" -> "); Serial.println(dato[k]);
                    buff = strchr(buff, TOKEN) + 1;
                }
                procesaComandos();
                i = -(j + 1);
                memset(CadenaAscii, 0, LEN_CADENA * sizeof(char));
            }
        }
        i += rd_cnt;
    }
}
 
/**********************************************************************************************
***********************************************************************************************
*****************************************FUNCIONES*********************************************
***********************************************************************************************
**********************************************************************************************/
 
void procesaComandos(void)
{
  int cmd_flag = dato[TK_COMANDO];
 
  if(cmd_flag & CMD_MOVERMOTOR)
  {
      MoveMotor(dato[TK_SPEEDMOTOR], dato[TK_DIRMOTOR], dato[TK_NUMSTEP], dato[TK_FACTORACELERATION]);
  }
  if(cmd_flag & CMD_STOPMOTOR)
  {
      stopMotor();
  }
  if(cmd_flag & CMD_SAVEPOSITION)
  {
      //savePosition();
  }
  if(cmd_flag & CMD_GO_POSITION)
  {
      //goPosition(dato[TK_GOPOSITION]);
  }
  if(cmd_flag & CMD_SET_CONFIG)
  {
      //setConfig(DEFAULT_BACKLASS, DEFAULT_TYPEMOTOR, DEFAULT_MICROSTEP, DEFAULT_SCREWSTEP);
  }
  if(cmd_flag & CMD_GO_HOME)
  {
      goHome();
  }
  if(cmd_flag & CMD_STACKED)
  {
 
  }
 
}
//-----------------------------------------------------------------------------------------------------
 
void MoveMotor(int Dir, int NumSteps, int SpeedMax, int Aceleration)
{
    digitalWrite(PIN_DIR, Dir > 0 ? HIGH : LOW);
 
 
 
 
    int microsecs_fullStep = SpeedMin_Default;
 
    for(int i = 0; i < NumSteps; ++i)
    {
      if(microsecs_fullStep < SpeedMax)
        {
           microsecs_fullStep = microsecs_fullStep - Aceleration;
 
        }
 
       digitalWrite(PIN_STEP, HIGH);
       delayMicroseconds(microsecs_fullStep);
       digitalWrite(PIN_STEP, LOW);
       delayMicroseconds(microsecs_fullStep);
 
 
    }
 
    position += ((Dir > 0 ? 1 : -1) * 200 * NumSteps);
    // actualizamos la eeprom diciendo que el valor almacenado de position no es correcto (acabamos de movernos)
   // eepromSaveVerify(EEPROM_VERIFY_FAIL);
}
//-----------------------------------------------------------------------------------------------------
 
void stopMotor()
{
}
//-----------------------------------------------------------------------------------------------------
 
void goHome()
{
 
  digitalWrite(13, HIGH);
  /*stepperResolution(DEFAULT_MICROSTEP);
  //char pinStop = digitalRead(PIN_ENDSTOP
  while (digitalRead(PIN_ENDSTOP))
  {
      MoveMotor(DIR_NEG_STEP,STEP_UNIT, DEFAULT_SPEED, DEFAULT_ACELERATION);
 
 
  }
  EEPROM.write(EEPROM_ADDR_POSITION,     0x00);
  EEPROM.write(EEPROM_ADDR_POSITION + 1, 0x00);
  EEPROM.write(EEPROM_ADDR_POSITION + 2, 0x00);
  EEPROM.write(EEPROM_ADDR_POSITION + 3, 0x00);
  //EEPROM.write(EEPROM_ADDR_VERIFY, EEPROM_VERIFY_SUCCESS);
  position = 0;
  //StackBlue.write(0);*/
 
}
//-----------------------------------------------------------------------------------------------------
 
/*int numSteps()
{
  float aux1 = Steps_Revolution * dato[TK_USERDISTANCE] / Get_ScrewStep;
  int numSteps = round(aux1);
 
  return numSteps;
}*/
//-----------------------------------------------------------------------------------------------------
 
void stepperResolution(char resolution)
{
  if(resolution == MICROSTEPS_SIXTEENTH)
  {
   digitalWrite(PIN_MS0, LOW);
   digitalWrite(PIN_MS1, LOW);
   digitalWrite(PIN_MS2, HIGH);
  }
  if(resolution == MICROSTEPS_EIGHTH)
  {
   digitalWrite(PIN_MS0, HIGH);
   digitalWrite(PIN_MS1, HIGH);
   digitalWrite(PIN_MS2, LOW);
  }
  if(resolution == MICROSTEPS_QUARTER)
  {
   digitalWrite(PIN_MS0, LOW);
   digitalWrite(PIN_MS1, HIGH);
   digitalWrite(PIN_MS2, LOW);
  }
  if(resolution == MICROSTEPS_HALF)
  {
   digitalWrite(PIN_MS0, HIGH);
   digitalWrite(PIN_MS1, LOW);
   digitalWrite(PIN_MS2, LOW);
  }
  if(resolution == MICROSTEPS_FULL)
  {
   digitalWrite(PIN_MS0, LOW);
   digitalWrite(PIN_MS1, LOW);
   digitalWrite(PIN_MS2, LOW);
  }
}
//-----------------------------------------------------------------------------------------------------
 
 
 
/*void stacked()
{
 goPosition(dato[TK_STARTSTACKED]);
 
 
 
 
            /*  dato[TK_ENDSTACKED],
              dato[TK_DIRMOTOR],
              dato[TK_USERDISTANCE],
              dato[TK_NUMPHOTOS],
              dato[TK_TIMEAFTERSHOOT],
              dato[TK_DISTANCEPHOTO],
              dato[TK_ENDSTACKED],
              dato[TK_ENDSTACKED],
              dato[TK_ENDSTACKED],
              dato[TK_ENDSTACKED],
              dato[TK_ENDSTACKED],*/
 
//}
//-----------------------------------------------------------------------------------------------------
 
int distance()
{
}
//-----------------------------------------------------------------------------------------------------
 
int setStarStacked(int setStart)
{
}
//-----------------------------------------------------------------------------------------------------
 
void goPosition(int goPosition)
{
    int dir = dato[TK_GOPOSITION] > position ? DIR_POS_STEP : DIR_NEG_STEP;
    while(goPosition != position)
    {
        int Acc;
        MoveMotor(DEFAULT_SPEED, dir, STEP_UNIT, dato[TK_FACTORACELERATION]);
    }
}
//-----------------------------------------------------------------------------------------------------
 
/*void getConfig()
{
  Get_BackLass  = EEPROM.read(EEPROM_ADDR_BACKLASS);
  Get_TypeMotor = ((int)EEPROM.read(EEPROM_ADDR_TYPEMOTOR)) * 10;
  Get_MicroStep = EEPROM.read(EEPROM_ADDR_MICROSTEP);
  Get_ScrewStep = ((int)EEPROM.read(EEPROM_ADDR_SCREWSTEP)) | (((int)EEPROM.read(EEPROM_ADDR_SCREWSTEP + 1)) << 8);
 
  Steps_Revolution = Get_TypeMotor * Get_MicroStep;
 
  Dist_Step = Get_ScrewStep / Steps_Revolution;
}*/
//-----------------------------------------------------------------------------------------------------
 
/*void setConfig(int BackLass, int TypeMotor, int MicroStep, int ScrewStep)
{
  EEPROM.write(EEPROM_ADDR_BACKLASS, BackLass);
  EEPROM.write(EEPROM_ADDR_TYPEMOTOR, TypeMotor / 10);
  EEPROM.write(EEPROM_ADDR_MICROSTEP, MicroStep);
  EEPROM.write(EEPROM_ADDR_SCREWSTEP, (char)ScrewStep);
  EEPROM.write(EEPROM_ADDR_SCREWSTEP + 1, (char)(ScrewStep >> 8));
 
  getConfig();
}*/
//-----------------------------------------------------------------------------------------------------
 
void eepromSaveVerify(unsigned char valor)
{
    if(verify_save != valor)
    {
        verify_save = valor;
        EEPROM.write(EEPROM_ADDR_VERIFY, verify_save);
    }
}
//-----------------------------------------------------------------------------------------------------
 
void savePosition()
{
  if(dato[TK_SAVEPSOTION]==1)
  {
      EEPROM.write(EEPROM_ADDR_POSITION, position);
      EEPROM.write(EEPROM_ADDR_POSITION + 1, (char)(position >> 8));
      EEPROM.write(EEPROM_ADDR_POSITION + 2, (char)(position >> 16));
      EEPROM.write(EEPROM_ADDR_POSITION + 3, (char)(position >> 24));
      eepromSaveVerify(EEPROM_VERIFY_SUCCESS);
 
  }
}

