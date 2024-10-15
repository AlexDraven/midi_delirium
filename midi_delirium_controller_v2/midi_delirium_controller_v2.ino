#include <MIDIUSB.h>

const int debugLedPin = 13;
const int ledPin = 9;
const int buttonPin = 2;

byte randNote = 0;
int buttonState = LOW;
int buttonPState = LOW;  // Estado anterior del botón

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10; 
int BUTTON_CH = 0; // Canal MIDI para eventos del botón

void setup() {
  Serial.begin(9600);
  
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT); // Configurar el pin del LED como salida
}

void loop() {
  buttons();
}

void buttons() {
  int reading = digitalRead(buttonPin);

 
  if (reading != buttonPState) {
    lastDebounceTime = millis(); 
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        randNote = random(22, 108); // Generar nota aleatoria entre 22 y 107
        noteOn(BUTTON_CH, randNote, 127);
        MidiUSB.flush();

        Serial.print("Button on: Note ");
        Serial.println(randNote);
        digitalWrite(ledPin, HIGH); 

      } else {
        for (int velocity = 127; velocity > 0; velocity -= 10) { 
          noteOff(BUTTON_CH, randNote, velocity); // Enviar MIDI Note Off con velocidad decreciente
          MidiUSB.flush();
          delay(20); 
        }
        noteOff(BUTTON_CH, randNote, 0);
        MidiUSB.flush();

        Serial.print("Button off: Note ");
        Serial.println(randNote);
        digitalWrite(ledPin, LOW); // Apagar el LED al soltar
      }
    }
  }

  buttonPState = reading; // Guardar el estado anterior del botón
}

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = { 0x08, 0x80 | channel, pitch, velocity };
  MidiUSB.sendMIDI(noteOff);
}
