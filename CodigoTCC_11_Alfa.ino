////////////////////////////////////////////  IMPORTAR BIBLIOTECAS  ////////////////////////////////////////////
#include <LiquidCrystal_I2C.h>                					
#include <Adafruit_MCP23X17.h>

////////////////////////////////////////////   INSTANCIAR OBJETOS //////////////////////////////////////////////
LiquidCrystal_I2C lcd(0x27, 20, 4);         //LCD
Adafruit_MCP23X17  mcp, mcp1, mcp2, mcp3;    									// MCP23017 0 1 2 3

////////////////////////////////////////////    DECLARAR I/Os   ////////////////////////////////////////////
const byte botup = 2;           				    //botão up
const byte botenter = 4;										//botão entra
const byte botdown = 5;            	        //botão down
const byte botback = 18;            		    //botão back
const byte leitura = 34;							      //leitura do cabo
const byte buzzer = 13;                     //buzzer
const byte binarioB1 = 32;                  //contagem binaria tipo de adaptador no lado A bit 1
const byte binarioB2 = 33;                  //contagem binaria tipo de adaptador no lado A bit 2
const byte binarioB3 = 25;                  //contagem binaria tipo de adaptador no lado A bit 3
const byte binarioA1 = 26;                  //contagem binaria tipo de adaptador no lado B bit 1
const byte binarioA2 = 27;                  //contagem binaria tipo de adaptador no lado B bit 2
const byte binarioA3 = 14;                  //contagem binaria tipo de adaptador no lado B bit 3

////////////////////////////////////////////   DECLARAR VARIAVEIS   ////////////////////////////////////////////
volatile bool botcontrol = 1;                                                             //controle do botão (1 sinal)
volatile bool status = 1;                                                                 //resultado aprovado = 1 e reprovado = 0
unsigned int numerotela = 2;				  					                                          //variavél que controla a tela
unsigned char numerocabo = 0;                 	                                          //numero do cabo usado internamenteno software
unsigned char qtdvias = 0;                    	                                          //quantidade de vias do cabo
unsigned char awg = 0;						  					                                            //bitola do cabo
const unsigned char comprimentoArray[8] = {0, 1, 2, 5, 10, 20, 50, 100};				  				//comprimentos do cabo
unsigned char comprimento = 0;
volatile unsigned int FiosReceita[11][11];					  	                                          //relação das vias do cabo da receita
volatile unsigned int FiosTeste[11][11];					  		                                          //relação das vias do cabo testado
volatile bool binarioreceitaA1, binarioreceitaA2, binarioreceitaA3;                       //codigo binario dos adaptadores
volatile bool binarioreceitaB1, binarioreceitaB2, binarioreceitaB3;                       //codigo binario dos adaptadores
volatile bool adapOK = 0;                                                                 //adaptadorOK = 1 NOK = 0
volatile bool reteste = 0;                                                                //verifica se ja ocorreu um teste
volatile bool simetrico = 0;                                                              //verifica se ja ocorreu um teste
const unsigned char dly150 = 150;                                                         //delay padrão 50
const unsigned char dly50 = 50;                                                           //delay padrão 50
const unsigned char dly20 = 20;                                                           //delay padrão 50
const unsigned char dly10 = 10;                                                           //delay padrão 10

///////////////////////////////////////////   VARIAVEIS CALCULO    ////////////////////////////////////////////
const unsigned int resolucao = 4095;
const float Vinmax = 3.3; 									              //tensão máxima de entrada do microcontrolador
const float VCEsat = 0.16;
const float Resgiga = 0.4;
volatile float Vin, Rmed, Rcal;              				  						//resistencia nominal do cabo (Calculado)
const unsigned char RC = 10;     //resistencia da giga 2x0.5 + Resistor do coletor     

////////////////////////////////////////////   INICIO VOID SETUP   ////////////////////////////////////////////
void setup() {                               
  lcd.init();                             						//inicia LCD
  lcd.backlight();                        						//inicia backlight
  pinMode(botup, INPUT);            					      	//declara modo de operação do pino do botão "up"
  pinMode(botenter, INPUT);               						//declara modo de operação do pino do botão "enter"
  pinMode(botdown, INPUT);            			      		//declara modo de operação do pino do botão "down"
  pinMode(botback, INPUT);            			      		//declara modo de operação do pino do botão "back"
  pinMode(leitura, INPUT);            			      		//declara modo de operação do pino "leitura"
  pinMode(buzzer, OUTPUT);            			      			//declara modo de operação do pino "leitura"
  pinMode(binarioA1, INPUT);            			      	//declara modo de operação do pino "leitura"
  pinMode(binarioA2, INPUT);            			      	//declara modo de operação do pino "leitura"
  pinMode(binarioA3, INPUT);            			      	//declara modo de operação do pino "leitura"
  pinMode(binarioB1, INPUT);            			      	//declara modo de operação do pino "leitura"
  pinMode(binarioB2, INPUT);            			      	//declara modo de operação do pino "leitura"
  pinMode(binarioB3, INPUT);            			      	//declara modo de operação do pino "leitura"
  Serial.begin(9600);                   					  	//inicia serial usado para debug
  inicio();                             					   	//chama a função inicio
  while (!Serial);
  if(!mcp.begin_I2C(0x22)) Serial.println("Erro na Inicialização MCP1");                            //testa se os MCPs inicializaram
  if(!mcp1.begin_I2C(0x21))Serial.println("Erro na Inicialização MCP2");	                          //testa se os MCPs inicializaram
  if(!mcp2.begin_I2C(0x23)) Serial.println("Erro na Inicialização MCP3");	                          //testa se os MCPs inicializaram
  if(!mcp3.begin_I2C(0x24)) Serial.println("Erro na Inicialização MCP4");	                          //testa se os MCPs inicializaram
  if(!mcp3.begin_I2C(0x24) && !mcp2.begin_I2C(0x23) && !mcp.begin_I2C(0x22) && !mcp1.begin_I2C(0x21)) while(1);
  for(int i = 0; i <= 15; i++) {							      	//declara modo de operação dos pinos do MCP	
    mcp3.pinMode(i, OUTPUT); mcp2.pinMode(i, OUTPUT); mcp1.pinMode(i, OUTPUT); mcp.pinMode(i, OUTPUT);
  }
}                                            
////////////////////////////////////////////    FIM VOID SETUP    ////////////////////////////////////////////

////////////////////////////////////////////   INICIO VOID LOOP   ////////////////////////////////////////////
void loop() {
  if(!digitalRead(botdown) && !digitalRead(botup) && !digitalRead(botenter) && !digitalRead(botback) && !botcontrol)botcontrol = 1;
  else{
   if(numerotela % 2 == 0) telas();  						            //testa se numerotela é par e chama a função telas
     else{                                   				        //testa se numerotela é impar e permite pressionar botões
       if(botcontrol && (digitalRead(botdown) || digitalRead(botenter) || digitalRead(botback) || digitalRead(botup))){
         if(digitalRead(botdown)){				      //testa se o botão apertado foi o "down"
           if((numerotela > 0 && numerotela <6) || (numerotela > 32 && numerotela <50) || (numerotela > 310 && numerotela <322) || (numerotela > 3110 && numerotela < 3122)){
              numerotela++;						   //soma 1 ao valor atual da tela
            }
            else if(numerotela == 73 && qtdvias > 1){          //controle de soma da quantidade de vias teste manual
              qtdvias--; numerotela--;
            }
          }
         else if(digitalRead(botup)){		           //testa se o botão apertado foi o "up"
           if(numerotela > 34 && numerotela < 50)numerocabo = numerocabo - 2;                       //subtrai 2 do valor do numero do cabo
           else if(numerotela > 312 && numerotela < 324)awg = awg - 4;                              //subtrai 4 do valor do Awg
           else if(numerotela > 3112 && numerotela < 3124)comprimento = comprimento - 2;            //subtrai 2 do valor do comprimento
           if((numerotela > 4 && numerotela < 8) || (numerotela > 34 && numerotela < 50) || (numerotela > 312 && numerotela < 324) || (numerotela > 3112 && numerotela < 3124)){
             numerotela = numerotela - 3;							         //subtrai 3 ao valor atual da tela
           }
            else if(numerotela == 73 && qtdvias < 10){           //controle de soma da quantidade de vias teste manual
             qtdvias++; numerotela--;
             Serial.println(qtdvias);
            }
          } 
         else if(digitalRead(botenter))numerotela = numerotela * 10;   //testa se o botão apertado foi o "enter" e multiplica por 10 o valor atual da tela        }
         else if(digitalRead(botback) && numerotela > 32){
          numerotela--; numerotela = numerotela / 10; numerotela--;
         }
         buzzerfunc(1);
         botcontrol = 0;										       //zera o bot control para não entrar num loop
       }
     }
  }
}
////////////////////////////////////////////     FIM VOID LOOP    ////////////////////////////////////////////

////////////////////////////////////////////  INICIO VOID TESTEAUTO  ////////////////////////////////////////////
void testeauto(){
  Rcal = 0.0168*((float)comprimento/((3.14*pow((0.127*pow(92, ((36-(float)awg)/39))), 2))/4)); 		//resistencia calculada
  delay(dly10);
  for(unsigned char x = 0; x <= (qtdvias - 1); x++){
    //Serial.println("");
    mcp1.digitalWrite(x, HIGH);	mcp3.digitalWrite(x, HIGH);																    	//liga saida do MCP1 e MCP3
    for(unsigned char y = 0; y <= (qtdvias -1); y++){    
      mcp.digitalWrite(y, HIGH);	mcp2.digitalWrite(y, HIGH);																  //liga saida do MCP0 e MCP2
      delay(dly10);
      Vin = (analogRead(leitura)*(Vinmax/resolucao));
      delay(dly10);
      Rmed = (RC*((((Vinmax-(2*VCEsat))/(Vinmax-Vin))-1)));
      Rmed = Rmed - (2*Resgiga);
      delay(dly10);
      if (Vin > 0.0 && Rmed <= (4*Rcal))FiosTeste[x][y] = 1;																		//testa se a resistencia do cabo é menor ou igual a nominal calculada
      else if (Vin > 0.0 && Rmed > (4*Rcal))FiosTeste[x][y] = 2;	
      else FiosTeste[x][y] = 0;			        																			          //testa se a resistencia do cabo é menor ou igual a nominal calculada
      delay(dly10);
      Serial.print("Vin: ");
      Serial.print(Vin);
       Serial.print("  Rmed: ");
      Serial.print(Rmed);
      Serial.print("   Rcal: ");
      Serial.println(Rcal);
      Serial.print(FiosTeste[x][y]);
      mcp.digitalWrite(y, LOW);	mcp2.digitalWrite(y, LOW);																  //desliga saida do MCP0 e MCP2
    }
   mcp1.digitalWrite(x, LOW);	mcp3.digitalWrite(x, LOW);																    	//desliga saida do MCP1 e MCP3
  }
  //Serial.println(status);
}
////////////////////////////////////////////   FIM VOID TESTEAUTO  ////////////////////////////////////////////

//////////////////////////////////////////// INICIO VOID RESULTADO ////////////////////////////////////////////
void resultado(){		
  status = 1;	botcontrol = 1;				
  for(unsigned char x = 0; x <= (qtdvias - 1); x++){ 
    delay(dly10);   
    for(unsigned char y = 0; y <= (qtdvias -1); y++){   
      delay(dly10);
      if (FiosTeste[x][y] != FiosReceita[x][y]){  
        mcp3.digitalWrite(x, HIGH);
        mcp2.digitalWrite(y, HIGH);
        lcd.setCursor(0, 0); lcd.print("Ligacao entre:      ");
        lcd.setCursor(0, 1); lcd.print("Pin "+((String)(x+1))+" LA e Pin "+((String)(y+1))+" LB ");
        lcd.setCursor(0, 2); 
        if ((FiosTeste[x][y] == 1) && (FiosReceita[x][y] == 0)) lcd.print("EM CURTO            ");
        else if ((FiosTeste[x][y] == 2) && (FiosReceita[x][y] == 1)) lcd.print("COM ALTA RESISTENCIA");
        else if ((FiosTeste[x][y] == 0) && (FiosReceita[x][y] == 1)) lcd.print("ROMPIDA             ");
        lcd.setCursor(0, 3); lcd.print("                    ");
        delay(750);
        mcp3.digitalWrite(x, LOW);
        mcp2.digitalWrite(y, LOW);
        status = 0;
      }
    }
    if(digitalRead(botenter) && botcontrol){			   //testa se o botão apertado foi o "enter"
        botcontrol = 0; break;	  					             //zera o bot control para não entrar num loop
    }
  }
  Serial.println("");
  for(unsigned char x = 0; x <= (qtdvias - 1) ; x++){
    for(unsigned char y = 0; y  <= (qtdvias - 1); y++){
      Serial.print(FiosReceita[x][y]);
    }
    Serial.println("");
  }
  Serial.println("");
  for(unsigned char x = 0; x <= (qtdvias - 1) ; x++){
    for(unsigned char y = 0; y  <= (qtdvias - 1); y++){
      Serial.print(FiosTeste[x][y]);
    }
    Serial.println("");
  }
  Serial.println("");
}
/////////////////////////////////////////////// INICIO VOID RESULTADO /////////////////////////////////////////////

////////////////////////////////////////////  INICIO VOID TESTEMANUAL  ////////////////////////////////////////////
void testemanual(){
  delay(dly150);
  char cont = 0;
  status = 1; 
  for(unsigned char x = 9;cont < 10; x--){
    if(x <= (9 - qtdvias))cont = 11;
    else{
      botcontrol = 1; 
      mcp3.digitalWrite(cont, HIGH);	mcp2.digitalWrite(cont, HIGH);	mcp1.digitalWrite(x, HIGH);	mcp.digitalWrite(x, HIGH);																	//liga saida do MCPs
      Vin = (analogRead(leitura)*(Vinmax/resolucao));
      delay(dly50);
      Serial.println(Vin);
      if (Vin > 0.0 && Vin < 1000) FiosTeste[x][x] = 1;																		//testa se a resistencia do cabo é menor ou igual a nominal calculada e armazena resultado no array
		  else FiosTeste[x][x] = 0;       																						             //testa se a resistencia do cabo é menor ou igual a nominal calculada e armazena resultado no array
      delay(dly50);
      lcd.setCursor(0, 0); lcd.print("Ligacao entre:      ");
      lcd.setCursor(0, 1); lcd.print("Pin "+((String)(cont+1))+" LA e Pin "+((String)(cont+1))+" LB ");  
      lcd.setCursor(0, 3); lcd.print("--------------------");
      lcd.setCursor(0, 2); 
      if (FiosTeste[x][x] != 1){     
        status = 0;
        lcd.print("NOK              ;-;");
      }
      else lcd.print("OK                :)");
      while(1){
        if(digitalRead(botenter) && botcontrol){			   //testa se o botão apertado foi o "enter"
          botcontrol = 0; break;	  					             //zera o bot control para não entrar num loop
        }
      }
    }
    mcp3.digitalWrite(cont, LOW); mcp2.digitalWrite(cont, LOW);	 mcp1.digitalWrite(x, LOW); mcp.digitalWrite(x, LOW);															  		//desliga saida do MCPs
    cont ++;
    delay(dly50);
  }
}
////////////////////////////////////////////   FIM VOID TESTEAUTO  ////////////////////////////////////////////

//////////////////////////////////////////// INICIO VOID RECEITAS ////////////////////////////////////////////
void receitas(){
  switch (numerocabo){
   case 1:														//receita do cabo ethernet Straight Through
      qtdvias = 8; simetrico = 1;
      binarioreceitaA1 = 0; binarioreceitaA2 = 0; binarioreceitaA3 = 1; binarioreceitaB1 = 0; binarioreceitaB2 = 0; binarioreceitaB3 = 1;
      FiosReceita [0][0] = 1; FiosReceita [1][1] = 1; FiosReceita [2][2] = 1; FiosReceita [3][3] = 1; 
      FiosReceita [4][4] = 1; FiosReceita [5][5] = 1; FiosReceita [6][6] = 1; FiosReceita [7][7] = 1;
   break;
   case 2:														//receita do cabo ethernet Crossover
      qtdvias = 8; simetrico = 1;
      binarioreceitaA1 = 0; binarioreceitaA2 = 0; binarioreceitaA3 = 1; binarioreceitaB1 = 0; binarioreceitaB2 = 0; binarioreceitaB3 = 1;
      FiosReceita [2][4] = 1; FiosReceita [4][2] = 1; FiosReceita [6][7] = 1; FiosReceita [7][6] = 1; 
      //FiosReceita [4][4] = 1; FiosReceita [5][1] = 1; FiosReceita [6][6] = 1; FiosReceita [7][7] = 1;
   break;
   case 3:														//receita do cabo Profinet RJ45 para M12
      qtdvias = 8; simetrico = 0;
      binarioreceitaA1 = 0; binarioreceitaA2 = 0; binarioreceitaA3 = 1; binarioreceitaB1 = 1; binarioreceitaB2 = 0; binarioreceitaB3 = 0;
      FiosReceita [0][0] = 1; FiosReceita [1][2] = 1; FiosReceita [2][1] = 1; FiosReceita [3][5] = 1; 
   break;
   case 4:														//receita do cabo Profinet RJ45
      qtdvias = 8; simetrico = 1;
      binarioreceitaA1 = 0; binarioreceitaA2 = 0; binarioreceitaA3 = 1; binarioreceitaB1 = 0; binarioreceitaB2 = 0; binarioreceitaB3 = 1;
      FiosReceita [2][2] = 1; FiosReceita [4][4] = 1; FiosReceita [6][6] = 1; FiosReceita [7][7] = 1; 
   break;
   case 5:														//receita do cabo Profinet M12
      qtdvias = 4; simetrico = 1;
      binarioreceitaA1 = 1; binarioreceitaA2 = 0; binarioreceitaA3 = 0; binarioreceitaB1 = 1; binarioreceitaB2 = 0; binarioreceitaB3 = 0;
      FiosReceita [0][0] = 1; FiosReceita [1][1] = 1; FiosReceita [2][2] = 1; FiosReceita [3][3] = 1; 
   break;
   case 6:														//receita do cabo RS232 Simple Null Modem Cable
      qtdvias = 8; simetrico = 1;
      binarioreceitaA1 = 0; binarioreceitaA2 = 1; binarioreceitaA3 = 0; binarioreceitaB1 = 0; binarioreceitaB2 = 1; binarioreceitaB3 = 0;
      FiosReceita [0][0] = 1; FiosReceita [1][1] = 1; FiosReceita [2][2] = 1; FiosReceita [3][3] = 1; 
      FiosReceita [4][4] = 1; FiosReceita [5][5] = 1; FiosReceita [6][6] = 1; FiosReceita [7][7] = 1;
   break;
   case 7:														//receita do cabo RS232 Null Modem Cable with Handshake
      qtdvias = 8; simetrico = 1;
      binarioreceitaA1 = 0; binarioreceitaA2 = 1; binarioreceitaA3 = 0; binarioreceitaB1 = 0; binarioreceitaB2 = 1; binarioreceitaB3 = 0;
      FiosReceita [1][2] = 1; FiosReceita [2][1] = 1; FiosReceita [3][5] = 1; FiosReceita [4][4] = 1; 
      FiosReceita [5][3] = 1; FiosReceita [6][7] = 1; FiosReceita [7][6] = 1;
   break;
   case 8:														//receita do cabo USB-A versão 2.0
      qtdvias = 4; simetrico = 1;
      binarioreceitaA1 = 1; binarioreceitaA2 = 1; binarioreceitaA3 = 0; binarioreceitaB1 = 1; binarioreceitaB2 = 1; binarioreceitaB3 = 0;
      FiosReceita [0][0] = 1; FiosReceita [1][1] = 1; FiosReceita [2][2] = 1; FiosReceita [3][3] = 1; 
   break;
   case 9:														//receita do cabo USB-A versão 3.0
      qtdvias = 8; simetrico = 1;
      binarioreceitaA1 = 1; binarioreceitaA2 = 1; binarioreceitaA3 = 0; binarioreceitaB1 = 1; binarioreceitaB2 = 1; binarioreceitaB3 = 0;
      FiosReceita [0][0] = 1; FiosReceita [1][1] = 1; FiosReceita [2][2] = 1; FiosReceita [3][3] = 1; 
      FiosReceita [4][4] = 1; FiosReceita [5][5] = 1; FiosReceita [6][6] = 1; FiosReceita [7][7] = 1;
   break;
  }
}
////////////////////////////////////////////  FIM VOID RECEITAS  ////////////////////////////////////////////

////////////////////////////////////////////  INICIO VOID PINOBINARIO  ////////////////////////////////////////////
void pinobinario(){
  if (digitalRead(binarioA1) == binarioreceitaA1 && digitalRead(binarioA2) == binarioreceitaA2 && digitalRead(binarioA3) == binarioreceitaA3 && digitalRead(binarioB1) == binarioreceitaB1 && digitalRead(binarioB2) == binarioreceitaB2 && digitalRead(binarioB3) == binarioreceitaB3) adapOK = 1;
}
////////////////////////////////////////////  FIM VOID PINOBINARIO ////////////////////////////////////////////

////////////////////////////////////////////  INICIO VOID BUZZERFUNC  ////////////////////////////////////////////
void buzzerfunc(char quant){
   for(unsigned char a = 0; a < quant; a++){
     digitalWrite(buzzer, HIGH); delay(dly50); digitalWrite(buzzer, LOW); delay(dly20);//Apito Buzzer
   }
}
////////////////////////////////////////////  FIM VOID BUZZERFUNC ////////////////////////////////////////////

////////////////////////////////////////////  INICIO VOID APROVACAO   ////////////////////////////////////////////
void aprovacao(){
    lcd.setCursor(0, 0); lcd.print("--------------------");
    lcd.setCursor(0, 1); lcd.print("       Cabo         ");
    lcd.setCursor(0, 2);
    if(status == 1){
      lcd.print("      Aprovado      ");
      lcd.setCursor(0, 3); lcd.print("--------------------");
       buzzerfunc(3);
    }
    else if(status == 0){
      lcd.print("      Reprovado     ");
      lcd.setCursor(0, 3); lcd.print("--------------------");
      buzzerfunc(6);
    }
    delay(4000);
}
////////////////////////////////////////////  FIM VOID PINOBINARIO ////////////////////////////////////////////

////////////////////////////////////////////  INICIO VOID RESET  ////////////////////////////////////////////
void reset(){													//reseta as variaveis de sistema
  qtdvias = 1; comprimento = 0; awg = 0; Rcal = 0.0; numerocabo = 0; status = 1; adapOK = 0; Vin = 0.0; Rmed = 0.0; Rcal = 0.0;
  binarioreceitaA1 = 0; binarioreceitaA2 = 0; binarioreceitaA3 = 0; binarioreceitaB1 = 0; binarioreceitaB2 = 0; binarioreceitaB3 = 0;
  for(unsigned char x = 0; x < 11; x++){
    for(unsigned char y = 0; y < 11; y++){
      FiosReceita[x][y] = 0; FiosTeste[x][y] = 0;
      mcp3.digitalWrite(y, LOW); mcp2.digitalWrite(y, LOW); mcp1.digitalWrite(y, LOW); mcp.digitalWrite(y, LOW);
    }
  }
}
////////////////////////////////////////////    FIM VOID RESET   ////////////////////////////////////////////

////////////////////////////////////////////  INICIO VOID TELAS  ////////////////////////////////////////////
void telas(){ 
  if(numerotela == 330 || numerotela == 350 || numerotela == 370 || numerotela == 390  || numerotela == 410 || numerotela == 430 || numerotela == 450 || numerotela == 470 || numerotela == 490)numerotela = 310;
 if(numerotela == 3130 || numerotela == 3150 || numerotela == 3170 || numerotela == 3190 || numerotela == 3210 || numerotela == 3220)numerotela = 3110;
 if(numerotela > 3300)numerotela = 2;						  //direciona a tela inicial
 if(numerotela == 2 || numerotela == 4  || numerotela == 6){      							                      //Tela Geral Inicio
    lcd.setCursor(0, 0); lcd.print("  Selecionar Receita");
    lcd.setCursor(0, 1); lcd.print("  Iniciar Teste Auto");
    lcd.setCursor(0, 2); lcd.print("  Iniciar Teste Man ");
    lcd.setCursor(0, 3); lcd.print("  Rct: "+(String)numerocabo+(String)+"  "+awg+"AWG  "+comprimentoArray[comprimento]+"m  ");
  }
  else if(numerotela == 30){    						                                                                  //tela N30 - reset
    reset(); numerotela = 31;
  }
  else if(numerotela == 32 || numerotela == 34 || numerotela == 36 || numerotela == 38){    						      //Tela Geral Receita
    lcd.setCursor(0, 0); 
    lcd.print("  EthernetST        ");
    lcd.setCursor(0, 1);
    lcd.print("  EthernetCR        ");
    lcd.setCursor(0, 2);
    lcd.print("  ProfinetRJ45-M12  ");
    lcd.setCursor(0, 3);
    lcd.print("  ProfinetRJ45   1/3");
    awg = 20;
    numerocabo ++;
  }
  else if(numerotela == 40 || numerotela == 42 || numerotela == 44 || numerotela == 46){    						      //Tela Geral Receita 2
    lcd.setCursor(0, 0); 
    lcd.print("  Sensor M12 5P     ");
    lcd.setCursor(0, 1);
    lcd.print("  RS232 SNMC        ");
    lcd.setCursor(0, 2);
    lcd.print("  RS232 NMCwHS      ");
    lcd.setCursor(0, 3);
    lcd.print("  USB-A v2.0     2/3");
    numerocabo ++;
  }
  else if(numerotela == 48){    						                                                               //tela N48 - Receita - point: receita USB-A versão 3.0
    lcd.setCursor(0, 0); 
    lcd.print("> USB-A v3.0        ");
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print("                 3/3");
    numerocabo ++;
  }
  else if (numerotela == 50){							                                                                  //tela N50 -  Testando auto
     receitas(); 
     pinobinario();
     lcd.setCursor(0, 0); lcd.print("--------------------");
     lcd.setCursor(0, 3); lcd.print("--------------------");
     lcd.setCursor(0, 1);
     if(numerocabo != 0 && awg != 0 && comprimento != 0 && adapOK == 1){	                                              //testa se EXISTE receita selecionada
       lcd.print("    Testando...     ");
       lcd.setCursor(0, 2);
       lcd.print("   *Nao Desligue*   ");
       delay(dly150);
       testeauto(); resultado(); aprovacao();
     }
     else{													                                                                    //testa se NÃO EXISTE receita selecionada
       for(unsigned char a = 0; a < 3; a++){				                                                    //função para piscar mensagem de "sem receita selecionada"
         buzzerfunc(1);
         if (numerocabo == 0 || awg == 20 || comprimento == 0 || awg == 0){
           lcd.setCursor(0, 1);
           lcd.print("     Sem Receita    ");
           lcd.setCursor(0, 2);
           lcd.print("     Selecionada    ");
         }
         else{
           lcd.setCursor(0, 1);
           lcd.print("      Adaptador     ");
           lcd.setCursor(0, 2);
           lcd.print("      Incorreto     ");
          }
          delay(500);										                                                                //tempo de cada piscada
          lcd.setCursor(0, 1);
          lcd.print("                    ");
          lcd.setCursor(0, 2);
          lcd.print("                    ");
          delay(250);
       }
     }
     numerotela = 1;	                                                                                //chama a tela inicial
  }
  else if (numerotela == 70){							                                                               //tela N70 -  Reset
    reset(); numerotela = 71;
  }
  else if (numerotela == 72){
    lcd.setCursor(0, 0); 
    lcd.print("--------------------");							                                                               //tela N72 -  Selecionando quantidade de vias
    lcd.setCursor(0, 1);
    lcd.print("Qtde de vias:       ");
    lcd.setCursor(15, 1);
    lcd.print(qtdvias);
    lcd.setCursor(0, 2);
    lcd.print("         Max 10 vias");
    lcd.setCursor(0, 3);
    lcd.print("--------------------");
  }
  else if(numerotela == 310 || numerotela == 312 || numerotela == 314 || numerotela == 316){            //Tela Geral Espessura 1
    lcd.setCursor(0, 0);
    lcd.print("  Ate 22AWG         ");
    lcd.setCursor(0, 1);
    lcd.print("  Ate 24AWG         ");
    lcd.setCursor(0, 2);
    lcd.print("  Ate 26AWG         ");
    lcd.setCursor(0, 3);
    lcd.print("  Ate 28AWG      1/2");
    awg = awg + 2;
  }
  else if(numerotela == 318 || numerotela == 320 || numerotela == 322){                                 //Tela Geral Espessura 2
    lcd.setCursor(0, 0);
    lcd.print("  Ate 30AWG         ");
    lcd.setCursor(0, 1);
    lcd.print("  Ate 32AWG         ");					  
    lcd.setCursor(0, 2);
    lcd.print("  Ate 34AWG         ");
    lcd.setCursor(0, 3);
    lcd.print("                 2/2");
    awg = awg + 2;
  }
   else if (numerotela == 730){							                                                           //tela N730 -  Chama o teste manual
    testemanual(); aprovacao(); numerotela = 1;
  }
  else if(numerotela == 3110  || numerotela == 3112 || numerotela == 3114 || numerotela == 3116){      //Tela Geral Comprimento 1
    lcd.setCursor(0, 0);
    lcd.print("  Ate 1m            ");
    lcd.setCursor(0, 1);
    lcd.print("  Ate 2m            ");
    lcd.setCursor(0, 2);
    lcd.print("  Ate 5m            ");
    lcd.setCursor(0, 3);
    lcd.print("  Ate 10m        1/2");
    comprimento ++;
  }
  else if(numerotela == 3118 || numerotela == 3120 || numerotela == 3122){    					        //tela N3118 - comprimento - point: ate 20 metros
    lcd.setCursor(0, 0);
    lcd.print("  Ate 20m           ");
    lcd.setCursor(0, 1);
    lcd.print("  Ate 50m           ");
    lcd.setCursor(0, 2);
    lcd.print("  Ate 100m          ");
    lcd.setCursor(0, 3);
    lcd.print("                 2/2");
    comprimento ++;
  }
  if(numerotela == 2 || numerotela == 32 || numerotela == 40 || numerotela == 310 || numerotela == 318 || numerotela == 3110 || numerotela == 3118)lcd.setCursor(0, 0);                                                                  //tela N32 - Receita - point: receita EthernetCST 
  else if(numerotela == 4 || numerotela == 34 || numerotela == 42 || numerotela == 312 || numerotela == 320 || numerotela == 3112 || numerotela == 3120)lcd.setCursor(0, 1);                                                             //tela N34 - Receita - point: receita EthernetCR
  else if(numerotela == 6 || numerotela == 36 || numerotela == 44 || numerotela == 314 || numerotela == 322 || numerotela == 3114 || numerotela == 3122)lcd.setCursor(0, 2);                                                             //tela N36 - Receita - point: receita ProfinetRJ45-M12
  else if(numerotela == 38 || numerotela == 46 || numerotela == 316 || numerotela == 3116)lcd.setCursor(0, 3);
  if(numerotela == 31 || numerotela == 71 || numerotela == 72)numerotela++;
  else{
    lcd.print(">"); numerotela++;
  }
}
////////////////////////////////////////////   FIM VOID TELAS   ////////////////////////////////////////////

////////////////////////////////////////////  INICIO VOID INICIO  ////////////////////////////////////////////
void inicio(){
  lcd.setCursor(0, 0); lcd.print("--------------------");
  lcd.setCursor(0, 1); lcd.print("   Giga de cabos    ");
  lcd.setCursor(0, 2); lcd.print("      FATEC-SP      ");
  lcd.setCursor(0, 3); lcd.print("--------------------");
  delay(3000);
}
////////////////////////////////////////////   FIM VOID INICIO   ////////////////////////////////////////////
