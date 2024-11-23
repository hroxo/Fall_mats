#include <LiquidCrystal_I2C.h>

// Inicializando o LCD I2C 
LiquidCrystal_I2C lcd(0x20,16,2);

// Definindo os pinos dos sensores, LEDs e piezo 
const int pirPin = A3;
const int forceSensorPin = A0;
const int redPin = 13;                // Pino do LED RGB (vermelho)
const int greenPin = 12;  
const int buttonPin = 3;
const int piezo = 2;

// Variáveis de estado
bool isResting = true;
bool isActive = false; 
bool isAlarm = false;
unsigned long lastMotionTime = 0;    // Variável para armazenar o tempo da última detecção de movimento
unsigned long lastForceCheckTime = 0; // Variável para controlar o delay de 2 segundos
unsigned long alarmStartTime = 0;    // Variável para armazenar o tempo em que o alarme começou
bool alarmTriggered = false;         // Verifica se o alarme foi acionado
float voltage;
float trueforce;

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(forceSensorPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);  // Botão configurado com resistor pull-up
  pinMode(piezo, OUTPUT);

  // Começar com os LEDs e piezo desligados
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(piezo, LOW);

  // Inicializar o Serial Monitor
  Serial.begin(9600);

  // Inicializar o LCD I2C
  lcd.init();
  lcd.clear();
  lcd.backlight();  // Ativa a luz de fundo do LCD
  // Print a message on both lines of the LCD.
  //lcd.cursor(column,row) index starts at 0
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("Estado: Repouso"); // Exibe o estado inicial no LCD
}

void loop() {
  // Ler o estado dos sensores
  int pirState = digitalRead(pirPin);
  int forceValue = analogRead(forceSensorPin); // Leitura do sensor de força
  int buttonState = digitalRead(buttonPin);    // Leitura do botão
  voltage = (forceValue * 5.0)/1023.0;
  trueforce = 2.1428 * pow(voltage, 2) - 0.6745* voltage +0.1398;
  
  // Registrar força no Serial Monitor a cada 2 segundos
  if (millis() - lastForceCheckTime >= 2000) {
    Serial.print("Força registrada: ");
    Serial.println(trueforce);
    lastForceCheckTime = millis();
  }
  
  // ESTADO DE REPOUSO
  if (isResting) {
    if (pirState == HIGH) {
      isResting = false;    // Sai do repouso
      isActive = true;      // Entra no estado ativo
      lastMotionTime = millis();
      digitalWrite(greenPin, HIGH); // Acende LED verde
      lcd.clear();
      lcd.print("Estado: Ativo");
      Serial.println("Estado: Ativo - Movimento detectado");
    }
    return; // Fica neste estado até detectar movimento
  }
  
  // ESTADO ATIVO
  if (isActive) {
    
    // Se o sensor de força registra um valor acima de 270, inicia o alarme
    if (trueforce >= 3) {
      isActive = false;
      isAlarm = true;        // Ativa o alarme
      alarmStartTime = millis();  // Armazena o tempo em que o alarme começou
      digitalWrite(greenPin, LOW); // Desliga o LED verde
      digitalWrite(redPin, HIGH);  // Liga o LED vermelho
      lcd.clear();
      lcd.print("Estado: Alarme");
      Serial.println("Estado: Alarme - Força detectada acima de 270");
    }
      
    if (pirState == HIGH) {
      lastMotionTime = millis(); // Reinicia o contador se houver movimento
      Serial.println("Movimento contínuo - Reiniciando temporizador");
    }
  }
  
  // ESTADO DE ALARME
  if (isAlarm) {
    // Verifica se 30 segundos se passaram desde o início do alarme
    unsigned long elapsedTime = millis() - alarmStartTime;
    if (elapsedTime < 15000) {
      // Atualiza o LCD com o tempo restante
      lcd.setCursor(0, 1);
      lcd.print("Restam: ");
      lcd.print((30000 - elapsedTime) / 1000);
      lcd.print(" seg");
    }

    if (elapsedTime >= 30000 && !alarmTriggered) {
      digitalWrite(piezo, HIGH);  // Ativa o piezo após 30 segundos
      alarmTriggered = true;
      lcd.clear();
      lcd.print("Alarme sonoro!");
      Serial.println("Alarme sonoro ativado após 30 segundos");
    }

    // Volta ao estado ativo ao pressionar o botão
    if (buttonState == LOW) { // O botão é pressionado
      isAlarm = false;
      isActive = true;
      alarmTriggered = false;  // Reseta o estado do alarme sonoro
      digitalWrite(redPin, LOW);  // Desliga o LED vermelho
      digitalWrite(greenPin, HIGH); // Liga o LED verde
      digitalWrite(piezo, LOW);  // Desliga o piezo
      lcd.clear();
      lcd.print("Estado: Ativo");
      Serial.println("Estado: Ativo - Botão pressionado, saindo do alarme");
    }
  }
}
