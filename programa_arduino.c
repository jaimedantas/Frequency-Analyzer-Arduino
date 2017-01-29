/*
  Foi usando a biblioteca em C escrita por Kevin Banks e baixada em
  http://www.embedded.com/design/embedded/4024443/The-Goertzel-Algorithm
  e foram modificadas alguns trechos do codigo para que a mesma funcionasse em 
  nosso programa. A partir de muita pesquisa na internet, identificamos partes chaves 
  que necessitaram ser mudadas para que criassemos um analisador de frequencia no Arduino
  para um range de frequencias, ja que inicialmente a biblioteca dele so funcionada para
  1 frequencia em especifico.
  Alunos: Jaime Dantas, Andres Rojas e Ramon Fava
*/
#include "LedControl.h"

#define MAXN 200
#define ADCCENTER 512

int sensorPin = A1;
int led2 = A3;
int relay = 10;
bool flag = false;
LedControl lc=LedControl(11,13,12,1);  // Pins: DIN,CLK,CS, # of Display connected
float led[8];

float SAMPLING_RATE;
float TARGET;
float N;
float coeff;
float Q1;
float Q2;
float sine;
float cosine;

byte testData[160];

//imprime a carinha
byte invader1a[] =
{
B00000000,
B01000010,
B00000000,
B00000000,
B01000010,
B00100100,
B00011000,
B00000000
};
// SAMPLING_FREQUENCY/N fica no centro do bin
// De acordo com Nyquist, a maior frequência que podemos usar é SAMPLING_FREQUENCY/2 

const int N = 115;   



const float SAMPLING_FREQUENCY = 9000; 


//funcoes modificadas da biblioteca --------------------------------------------------------


Goertzel(float TARGET_FREQUENCY,float BLOCK,float SAMPLING_FREQ)
{

  SAMPLING_RATE=SAMPLING_FREQ;  //on 16mhz, ~8928.57142857143, on 8mhz ~44444
  TARGET=TARGET_FREQUENCY; //must be integer of SAMPLING_RATE/N
  N=BLOCK;  //tamanho do bloco
  int k;
  float omega;

  k = (int) (N * (TARGET_FREQUENCY / SAMPLING_RATE) + 0.93);
  omega = (2.0 * PI * k) / N;
  sine = sin(omega);
  cosine = cos(omega);
  coeff = 2.0 * cosine;

  ResetGoertzel();
}


/* Call this routine before every "block" (size=N) of samples. */
void ResetGoertzel(void)
{
  Q2 = 0;
  Q1 = 0;
}


/* Call this routine for every sample. */
void ProcessSample(byte sample)
{
  float Q0;
  Q0 = coeff * Q1 - Q2 + (float) sample;
  Q2 = Q1;
  Q1 = Q0;
}


/* Basic Goertzel */
/* Call this routine after every block to get the complex result. */
void GetRealImag(float *realPart, float *imagPart)
{
  *realPart = (Q1 - Q2 * cosine);
  *imagPart = (Q2 * sine);
}


/* Sample some test data. */
void sample(int sensorPin)
{
  for (int index = 0; index < N; index++)
  {
    testData[index] = (byte) analogRead(0);
  }
}


float detect()
{

  int index;

  float magnitudeSquared;
  float magnitude;
  float real;
  float imag;

  /* Process the samples. */
  for (index = 0; index < N; index++)
  {
    ProcessSample(testData[index]);
  }

  /* Do the "standard Goertzel" processing. */
  GetRealImag(&real, &imag);

  magnitudeSquared = real*real + imag*imag;
  magnitude = (sqrt(magnitudeSquared)/2);

  ResetGoertzel();
  return magnitude;
}
//fim das funcoes modificadas ----------
//Inicio do processamento propriamente dito


void setup(){
  pinMode(led2, OUTPUT); 
  pinMode(relay, OUTPUT); 
  pinMode(A0, INPUT);    
  //Serial.begin(9600); 
  lc.shutdown(0,false);  
  lc.setIntensity(0,5);  
  lc.clearDisplay(0);  
  digitalWrite(led2, HIGH);
  digitalWrite(relay, LOW);
  inicio();
}
char normalizacao(float valor){
  char retorno = 0;
  if(valor>200){
    retorno += 1;
  }
  if(valor>210){
    retorno += 2;
  }
  if(valor>220){
    retorno += 4;
  }
  if(valor>260){
    retorno += 8;
  }
  if(valor>300){
    retorno += 16;
  }
  if(valor>350){
    retorno += 32;
  }
  if(valor>380){
    retorno += 64;
  }
  if(valor>400){
    retorno += 128;
  }
  return retorno;
}

void loop()
{
  
  //Serial.println(analogRead(sensorPin)); 
  for(int i=0; i<8; i++){
    Goertzel goertzel = Goertzel(156+156*i, N, SAMPLING_FREQUENCY);
    goertzel.sample(sensorPin); 
    led[i] = goertzel.detect();  
    //Serial.print(led[i]);
    //Serial.print("  ");
    char nome = normalizacao(led[i]);
    lc.setRow(0,i, reverse(nome));    
  }
      //Serial.println(analogRead(A0));


}
void ligarAudio(){
  digitalWrite(relay, HIGH);
  delay(10);
  //digitalWrite(relay, HIGH);
}

byte reverse(byte in) {
 byte out = 0;
 for(int i=0; i < 8; i++)
 out |= ((in >> i) & 1) << (7 - i);
 return out;
}
void inicio(){
  while(true){
    Goertzel goertzel = Goertzel(4000, N, SAMPLING_FREQUENCY);
    goertzel.sample(A0); 
    float ligar = goertzel.detect();  
    //Serial.println(ligar);
    if(ligar>900){
      digitalWrite(led2, LOW);
       for (int i = 0; i < 8; i++)  
         {
        lc.setRow(0,i,invader1a[i]);
        }
        delay(3000);
        ligarAudio();
      break;
    }
  }
}

