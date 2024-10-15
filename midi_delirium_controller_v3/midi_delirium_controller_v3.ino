#include <MIDIUSB.h>  // Librería que permite enviar mensajes MIDI a través del puerto USB

// Definir los pines para el botón y el LED
const int buttonPin = 2;   // Pin donde está conectado el botón
const int ledPin = 9;      // Pin donde está conectado el LED

// Variables para almacenar datos de la nota MIDI y el estado del botón
byte randNote = 0;         // Almacena la nota aleatoria que se generará
int buttonState = LOW;     // Estado actual del botón (presionado o no)
int buttonPState = LOW;    // Estado anterior del botón, para detectar cambios
bool notePlaying = false;  // Indica si una nota está actualmente sonando

// Variables para controlar la duración de la nota y el debounce del botón
unsigned long noteStartTime = 0;  // Marca de tiempo cuando la nota empezó a sonar
unsigned long noteDuration = 0;   // Duración de la nota en milisegundos
unsigned long lastDebounceTime = 0; // Marca de tiempo para controlar el rebote del botón
unsigned long debounceDelay = 10;   // Retraso de debounce en milisegundos (tiempo para evitar rebotes)
int BUTTON_CH = 0;  // Canal MIDI (usado para enviar las notas)

void setup() {
  Serial.begin(9600);  // Inicia la comunicación con la consola serie para depuración
  pinMode(buttonPin, INPUT_PULLUP);  // Configura el pin del botón como entrada con resistencia interna
  pinMode(ledPin, OUTPUT);           // Configura el pin del LED como salida
}

void loop() {
  buttons();          // Llama a la función para controlar el botón
  checkNoteDuration();  // Verifica si la nota debe dejar de sonar
}

// Esta función detecta cuando el botón es presionado o liberado
void buttons() {
  int reading = digitalRead(buttonPin);  // Lee el estado actual del botón

  // Si el estado del botón ha cambiado desde la última lectura, reinicia el temporizador de debounce
  if (reading != buttonPState) {
    lastDebounceTime = millis();  // Guarda el tiempo actual
  }

  // Si ha pasado más tiempo que el retraso de debounce, entonces actualizamos el estado del botón
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {  // Si el estado actual es diferente al anterior
      buttonState = reading;       // Actualiza el estado del botón

      // Si el botón está presionado (LOW), generamos o modificamos una nota
      if (buttonState == LOW) {
        if (!notePlaying) {  // Si no hay ninguna nota sonando actualmente
          randNote = random(22, 108);  // Genera una nota aleatoria entre 22 y 107
          noteOn(BUTTON_CH, randNote, 127);  // Envia un mensaje MIDI para iniciar la nota
          MidiUSB.flush();  // Envía inmediatamente los datos MIDI

          noteDuration = random(2000, 5000);  // Asigna una duración aleatoria a la nota (2 a 5 segundos)
          noteStartTime = millis();  // Guarda el tiempo actual como inicio de la nota
          notePlaying = true;  // Marca que una nota está sonando
          digitalWrite(ledPin, HIGH);  // Enciende el LED para indicar que la nota está activa

          // Imprime en la consola el número de la nota y su duración en segundos
          Serial.print("Note on: ");
          Serial.print(randNote);
          Serial.print(" for ");
          Serial.print(noteDuration / 1000.0);  // Convierte de milisegundos a segundos
          Serial.println(" seconds");

        } else {
          modifyNoteProperties();  // Si la nota está sonando, modificamos sus propiedades
        }
      }
    }
  }
  buttonPState = reading;  // Guarda el estado del botón para la próxima lectura
}

// Esta función revisa si la duración de la nota ya ha terminado
void checkNoteDuration() {
  if (notePlaying && (millis() - noteStartTime >= noteDuration)) {  // Si la nota está sonando y ha pasado su duración
    noteOff(BUTTON_CH, randNote, 0);  // Envía un mensaje MIDI para detener la nota
    MidiUSB.flush();  // Envía inmediatamente los datos MIDI
    digitalWrite(ledPin, LOW);  // Apaga el LED indicando que la nota ha terminado
    notePlaying = false;  // Marca que ya no hay ninguna nota sonando

    // Imprime en la consola que la nota se ha apagado
    Serial.print("Note off: ");
    Serial.println(randNote);
  }
}

// Esta función modifica las propiedades de la nota (tono y velocidad) mientras está sonando
void modifyNoteProperties() {
  byte newVelocity = random(30, 127);  // Genera una nueva velocidad (volumen) aleatoria
  byte newPitch = randNote + random(-2, 3);  // Ajusta la nota aleatoriamente dentro de un rango pequeño
  newPitch = constrain(newPitch, 22, 107);  // Asegura que el tono esté dentro del rango permitido

  noteOff(BUTTON_CH, randNote, 0);  // Apaga la nota actual
  noteOn(BUTTON_CH, newPitch, newVelocity);  // Envia un mensaje MIDI para tocar la nueva nota con el nuevo volumen
  MidiUSB.flush();  // Envía inmediatamente los datos MIDI

  randNote = newPitch;  // Actualiza la variable de la nota con el nuevo valor

  // Imprime en la consola los detalles de la modificación
  Serial.print("Note modified: Pitch ");
  Serial.print(newPitch);
  Serial.print(", Velocity ");
  Serial.println(newVelocity);
}

// Función para enviar una nota "on" (nota activada) como mensaje MIDI
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };  // Crea un paquete MIDI "Note On"
  MidiUSB.sendMIDI(noteOn);  // Envía el mensaje MIDI
}

// Función para enviar una nota "off" (nota desactivada) como mensaje MIDI
void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = { 0x08, 0x80 | channel, pitch, velocity };  // Crea un paquete MIDI "Note Off"
  MidiUSB.sendMIDI(noteOff);  // Envía el mensaje MIDI
}
