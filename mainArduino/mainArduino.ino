/*
FECHA: 17/06/2017
AUTOR: Barkalez
*/
/****************************************************************
-------------------------------INFO------------------------------
*****************************************************************
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
/****************************************************************
------------------------Urls de interest-------------------------
****************************************************************
    1-. https://www.luisllamas.es/motores-paso-paso-arduino-driver-a4988-drv8825/
   2-.  https://github.com/barkalez/StackBlueV5
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
//Parámetros recibidos por Bluetooth
#define TK_DEINICIOAFINAL          0
#define TK_POSICIONSTART           1
#define TK_POSICIONEND             2
#define TK_MICRASENTREFOTOS        3
#define TK_FACTORACELERATION       4
#define TK_VEL_INICIAL             5
#define TK_VEL_FINAL               6
#define TK_TIMEBEFORESHOOT         7
#define TK_TIMEAFTERSHOOT          8
#define TK_SPEEDMOTOR              9
#define TK_DIRMOTOR                10
#define TK_NUMSTEP                 11
#define TK_GOPOSITION              12
#define TK_SAVEPSOTION             13
#define TK_MICROSTEP               14
#define TK_ENERGYMOTOR             15
#define TK_COMANDO                 16  // Hexadecimal que indica función
#define NUMTOKENS                  17  // Número de Tokens en total
//Constanes con referencias a Funciones que si son iguales que TK_COMANDO se ejecutan
#define CMD_MOVERMOTOR             0x00000001    //   0000 0000 0000 0000 0000 0000 0000 0001
#define CMD_STACK                  0x00000002    //   0000 0000 0000 0000 0000 0000 0000 0010
#define CMD_SAVEPOSITION           0x00000004    //   0000 0000 0000 0000 0000 0000 0000 0100
#define CMD_STACKED                0x00000008    //   0000 0000 0000 0000 0000 0000 0000 1000
#define CMD_GO_POSITION            0x00000010    //   0000 0000 0000 0000 0000 0000 0001 0000
//                                 0x00000020    //   0000 0000 0000 0000 0000 0000 0010 0000
#define CMD_GO_HOME                0x00000040    //   0000 0000 0000 0000 0000 0000 0100 0000
#define CMD_ENERGYMOTOR            0x00000080    //   0000 0000 0000 0000 0000 0000 1000 0000
//Constantes de gestión de cadena
#define LEN_CADENA                 1024
#define TOKEN                      '\n'  //Caracter para separar los tokens
#define END_OF_MSSG                'T'   //Caracter para indicar el fin del mensaje enviado
//Constantes para configurar el MicroStep del Nema17

//Constantes para escribir y leer en la EEPROM del arduino
#define EEPROM_ADDR_POSITION        0
#define EEPROM_ADDR_VERIFY          (EEPROM_ADDR_POSITION + sizeof(position))
#define EEPROM_VERIFY_FAIL          0x00
#define EEPROM_VERIFY_SUCCESS       0xff
//Constantes para configurar los pines del Arduino
#define PIN_KEY                     2
#define PIN_STEP                    9
#define PIN_MS0                     4
#define PIN_MS1                     3
#define PIN_MS2                     5
#define PIN_RESET                   6
#define PIN_TX                      10
#define PIN_RX                      11
#define PIN_DIR                     8
#define PIN_ENDSTOP                 13
//Constantes para configurar NEMA17
#define MICROSTEPS_SIXTEENTH        16
#define MICROSTEPS_EIGHTH           8
#define MICROSTEPS_QUARTER          4
#define MICROSTEPS_HALF             2
#define MICROSTEPS_FULL             1
#define DEFAULT_ACELERATION         1
#define DEFAULT_SPEED_MAX           500
#define DEFAULT_SPEED_MIN           600
#define STEP_UNIT                   1
#define DIR_POS_STEP                1
#define DIR_NEG_STEP                0
#define DistanceMinimal             1.270000
#define Micras_Unit                 0.714375
/****************************************************************
------------------------Global Variables-------------------------------
****************************************************************/
long                                dato[NUMTOKENS];
char                                CadenaAscii[LEN_CADENA];
unsigned long                       position;
unsigned char                       verify_save;
double                              pos;

/****************************************************************
------------------------Bluetooth-------------------------------
****************************************************************/
//Se defina los pines TX y RX en la comunicación Bluetooth
SoftwareSerial StackBlue(PIN_TX, PIN_RX);
//Función envía la posición actual por Bluetooth al Smartphone
void sendPosition()
{
    pos = ((float)(position) * DistanceMinimal) / (float)MICROSTEPS_SIXTEENTH;
    //pos = position * 0.079375;
    //pos = 1.42875;
    //pos = pos * 1000000;
    //int(pos);
    StackBlue.println(pos);
    Serial.println(pos);
}
/****************************************************************
------------------------Void Setup-------------------------------
****************************************************************/
void setup()
{
//Configuracion de los pines del Arduino
  pinMode       (PIN_STEP,       OUTPUT);
  pinMode       (PIN_MS0,        OUTPUT);
  pinMode       (PIN_MS1,        OUTPUT);
  pinMode       (PIN_MS2,        OUTPUT);
  pinMode       (PIN_DIR,        OUTPUT);
  pinMode       (PIN_ENDSTOP,    INPUT);
  pinMode       (PIN_KEY,        OUTPUT);
      digitalWrite  (PIN_KEY,        HIGH);
  pinMode       (PIN_RESET,      OUTPUT);
      digitalWrite  (PIN_RESET,      HIGH);
//Configuración de la velocidad de conexión de StackBlue-Smartphone
  StackBlue.begin(9600);
//Configuración de la velocidad de conexión Serie-Pc
  Serial.begin(9600);
//Limpia la memoria SRAM que ocupa la array datos de datos anteriores.
  memset(dato, 0, NUMTOKENS * sizeof(int));
  memset(CadenaAscii, 0, LEN_CADENA * sizeof(char));
//Lee la posición de la última vez que se guardó en la EEPROM
//Lee la posición de la dirección EEPROM_ADDR_POSITION +1 y la desplaza 8 Bits a la izquierda y hace con ella un OR con lo que hay en la dirección EEPROM_ADDR_POSITION
  position    = ((unsigned long)(EEPROM.read(EEPROM_ADDR_POSITION + 3)) << 24)
              | ((unsigned long)(EEPROM.read(EEPROM_ADDR_POSITION + 2)) << 16)
              | ((unsigned long)(EEPROM.read(EEPROM_ADDR_POSITION + 1)) << 8)
              |  (unsigned long)EEPROM.read(EEPROM_ADDR_POSITION);
// Grabamos en EEPROM la condicion de fallo de grabáción de posición
  EEPROM.write(EEPROM_ADDR_VERIFY, EEPROM_VERIFY_FAIL);


//Asigna a verify_save el valor que lee de la EEPROM (EEPROM_ADDR_VERIFY)
  verify_save = EEPROM.read(EEPROM_ADDR_VERIFY);
//Si verify_save tiene el mismo valor que EEPROM_VERIFY_FAIL no se guardó correctamenta en el anterior encendido por lo tanto irá a Home y establecerá a 0 position
  /*if(verify_save == EEPROM_VERIFY_FAIL)
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
//Serial.print("Caracter "); Serial.print(j+1); Serial.print(": "); Serial.println(CadenaAscii[i+j]);
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
                    //dato[k] = atoi(aux);
                    dato[k] = String(aux).toInt();
Serial.print(aux); Serial.print("\" -> "); Serial.println(dato[k]);
                    buff = strchr(buff, TOKEN) + 1;
                }
                //procesaComandos();
                i = -(j + 1);
                memset(CadenaAscii, 0, LEN_CADENA * sizeof(char));
            }
        }
        i += rd_cnt;
    }
    procesaComandos();
}

/**********************************************************************************************
***********************************************************************************************
*****************************************FUNCIONES*********************************************
***********************************************************************************************
**********************************************************************************************/

void procesaComandos(void)
{
    //-- Guarda el estado anterior del comando CMD_MOVERMOTOR. Cuando pase de 1 a 0 se ha levantado el dedo del botón de mover el motor.
    static int lst_cmd_move = 0;
    int cmd_flag = dato[TK_COMANDO];

    //-- 1. PROCESA MOVER MOTOR
    //-- Si el comando me pide "muevete"... nos movemos
    if(cmd_flag & CMD_MOVERMOTOR)
    {
        MoveMotor(dato[TK_MICROSTEP], dato[TK_DIRMOTOR], dato[TK_NUMSTEP], dato[TK_VEL_INICIAL], dato[TK_VEL_FINAL], dato[TK_FACTORACELERATION]);
    }
    //-- Si no, miramos si justo antes sí que nos moviamos.
    else if(lst_cmd_move)
    {
        //-- Se acaba de levantar el dedo del botón de mover... reiniciamos el valor de la velocidad inicial a lo que
        //   tenemos en el MitApp (dato[TK_VEL_INICIAL])
        MoveMotor(0, 0, 0, dato[TK_VEL_INICIAL], -1, 0);
    }

    //-- 2. PROCESA SAVE POSITION
    if(cmd_flag & CMD_SAVEPOSITION)
    {
        //savePosition();
    }

    //-- 3. PROCESA GO TO POSITION
    if(cmd_flag & CMD_GO_POSITION)
    {
        goPosition(dato[TK_GOPOSITION]);
    }

    //-- 5. PROCESA GO HOME
    if(cmd_flag & CMD_GO_HOME)
    {
        goHome();
    }
    //-- 5. Energiza o desdenergiza el motor
    if(cmd_flag & CMD_ENERGYMOTOR)
    {
        digitalWrite(PIN_RESET, dato[TK_ENERGYMOTOR] > 0 ? HIGH : LOW);
    }
    //-- 6. PROCESA STACK
    if(cmd_flag & CMD_STACKED)
    {
        Stack();
    }

    //-- Limpiamos todos los comandos menos el de mover motor
    dato[TK_COMANDO] &= CMD_MOVERMOTOR;

    //-- Guardamos el estado actual del flag de MOVERMOTOR. Para poder detectar cuando cambia de estado
    lst_cmd_move = dato[TK_COMANDO] & CMD_MOVERMOTOR;
}
//-----------------------------------------------------------------------------------------------------
//void MoveMotor(int Dir, int NumSteps, int Speed, int Aceleration)
void MoveMotor(int MicroStep, int Dir, int NumSteps, int Velo_Inicial, int Velo_Final, int Aceleration)
{

    //-- GUardamos la velocidad que teníamos la vez anterior que llamaron a esta función.
    static int vel = Velo_Inicial;
    //-- Condición para que se resetee la velocidad inicial: Que Velo_Final sea negativa.
    if(Velo_Final < 0)
    {
        vel = Velo_Inicial;
        sendPosition();
        return;
    }

    if(dato[TK_ENERGYMOTOR] == 0) return;

    //-- Asignamos la dirección de movimiento
    digitalWrite(PIN_DIR, Dir > 0 ? HIGH : LOW);
    //-- Asignamos la resolución
    stepperResolution(MicroStep);

    //digitalWrite(PIN_RESET,HIGH);

    //int microsecs_fullStep = Velo_Inicial;

    //-- Bucle para ir acelerando el motor
    int i;  //-- almacenamos el numero de pasos que hemos dado
    for(i= 0; i< NumSteps; ++i)
    {
        if(vel > Velo_Final)
            vel -= Aceleration;

        digitalWrite(PIN_STEP, HIGH);
        delayMicroseconds(vel);
        digitalWrite(PIN_STEP, LOW);
        delayMicroseconds(vel);
    }

    //-- Calculamos la distancia en 1/16 de paso del carro desde el cero
   unsigned long pos_aux = (i * MICROSTEPS_SIXTEENTH) / dato[TK_MICROSTEP];
   position += (Dir > 0 ? 1 : -1) * pos_aux;
}
/************************************************************************************************************************
*************************************************************************************************************************
************************************************************************************************************************/
//***********************************************************************************************************************




void Stack()
{
  float MicrasEntreFotos  =  dato[TK_MICRASENTREFOTOS];
  MicrasEntreFotos        =  MicrasEntreFotos / 100;
  float PosicionStart     =  dato[TK_POSICIONSTART];
  PosicionStart           =  PosicionStart / 100;
  float PosicionEnd       =  dato[TK_POSICIONEND];
  PosicionEnd             =  PosicionEnd / 100;
  
  MicrasEntreFotos        = MicrasEntreFotos / Micras_Unit;
  MicrasEntreFotos        = round(MicrasEntreFotos) * Micras_Unit;
  PosicionStart           = PosicionStart / Micras_Unit;
  PosicionStart           = round(PosicionStart) * Micras_Unit;
  PosicionEnd             = PosicionEnd / Micras_Unit;
  PosicionEnd             = round(PosicionEnd) * Micras_Unit;
    
  float LongApilado       = PosicionEnd - PosicionStart;
  MicrasEntreFotos        = MicrasEntreFotos + 0.5;
  int FT                  = LongApilado / MicrasEntreFotos;
  FT = FT + 1;
  int NP                  = MicrasEntreFotos / Micras_Unit;
  int NPT                 = NP * FT;

Serial.println("Entra en funcion Stack");

Serial.println("Imprime TK_MICRASENTREFOTOS");
Serial.println(dato[TK_MICRASENTREFOTOS]);
Serial.println();
Serial.println("Imprime MicrasEntreFotos");
Serial.println(MicrasEntreFotos);
Serial.println();
Serial.println("Imprime TK_POSICIONSTART");
Serial.println(dato[TK_POSICIONSTART]);
Serial.println();
Serial.println("Imprime PosicionStart");
Serial.println(PosicionStart);
Serial.println();
Serial.println("Imprime TK_POSICIONEND");
Serial.println(dato[TK_POSICIONEND]);
Serial.println();
Serial.println("Imprime PosicionEnd");
Serial.println(PosicionEnd);
Serial.println();
Serial.println("LongApilado = PosicionEnd - PosicionStart");
Serial.println(LongApilado);
Serial.println();
Serial.println("Fotos = Mfr / Micras_Unit");
Serial.println(FT);
Serial.println("Numero de pasos entre fotos");
Serial.println(NP);
Serial.println("Numero de pasos totales");
Serial.println(NPT);













  /*if(dato[TK_DEINICIOAFINAL] == 1)
  {
Serial.println("Va de Inicio a Final");

        goPosition(PosicionStart);
        delayMicroseconds(dato[TK_TIMEBEFORESHOOT]);
        for(int i= 0; i< FT; ++i)
        {
Serial.println("Mueve motor x veces, esto se repite mucho");

         MoveMotor(dato[TK_MICROSTEP], DIR_POS_STEP, NPT, dato[TK_VEL_INICIAL], dato[TK_VEL_FINAL], dato[TK_FACTORACELERATION]);
         delayMicroseconds(dato[TK_TIMEBEFORESHOOT]);
         //Aquí va el disparo de la cámara de fotos
         //Aquí va el disparo de los flashes si hiciera falta
         delayMicroseconds(dato[TK_TIMEAFTERSHOOT]);

        }

  }
  if(dato[TK_DEINICIOAFINAL] == 0)
  {
Serial.println("Entra en TK_DEINICIOAFINAL 0");

        goPosition(PosicionEnd);
        delayMicroseconds(dato[TK_TIMEBEFORESHOOT]);

        for(int i= 0; i< FT; ++i)
        {
Serial.println("Mueve motor x veces, esto se repite mucho");
         MoveMotor(dato[TK_MICROSTEP], DIR_NEG_STEP, NPT, dato[TK_VEL_INICIAL], dato[TK_VEL_FINAL], dato[TK_FACTORACELERATION]);
         delayMicroseconds(dato[TK_TIMEBEFORESHOOT]);
         //Aquí va el disparo de la cámara de fotos
         //Aquí va el disparo de los flashes si hiciera falta
         delayMicroseconds(dato[TK_TIMEAFTERSHOOT]);
        }
  }*/

}

void goPosition(float destino)
{
Serial.println("Entra en goPosition");
sendPosition();
Serial.println(destino);
Serial.println(dato[TK_MICRASENTREFOTOS]);


  if(pos < destino)
    {
        while(pos < destino)
        {
          Serial.println(pos);
          Serial.println(destino);
          MoveMotor(MICROSTEPS_SIXTEENTH, DIR_POS_STEP,STEP_UNIT, DEFAULT_SPEED_MIN, DEFAULT_SPEED_MAX, DEFAULT_ACELERATION);
        }
    }
  else if(pos > destino)
    {
        while(pos < destino)
        {
          MoveMotor(MICROSTEPS_SIXTEENTH, DIR_NEG_STEP,STEP_UNIT, DEFAULT_SPEED_MIN, DEFAULT_SPEED_MAX, DEFAULT_ACELERATION);
        }
    }

}




















//************************************************************************************************************************
void goHome()
{
   while (digitalRead(PIN_ENDSTOP)== LOW)
      {
       MoveMotor(MICROSTEPS_FULL, DIR_NEG_STEP,STEP_UNIT, DEFAULT_SPEED_MIN, DEFAULT_SPEED_MAX, DEFAULT_ACELERATION);
      }


  EEPROM.write(EEPROM_ADDR_POSITION,     0x00);
  EEPROM.write(EEPROM_ADDR_POSITION + 1, 0x00);
  EEPROM.write(EEPROM_ADDR_POSITION + 2, 0x00);
  EEPROM.write(EEPROM_ADDR_POSITION + 3, 0x00);
  EEPROM.write(EEPROM_ADDR_VERIFY, EEPROM_VERIFY_SUCCESS);

  position =   EEPROM.read(EEPROM_ADDR_POSITION);

  sendPosition();
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
   digitalWrite(PIN_MS0, HIGH);
   digitalWrite(PIN_MS1, HIGH);
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
