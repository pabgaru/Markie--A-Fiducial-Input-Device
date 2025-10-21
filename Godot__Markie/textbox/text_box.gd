extends MarginContainer

@onready var label: Label = $MarginContainer/Label
@onready var timer: Timer = $MarginContainer/LetterDisplayTimer
@onready var hide_timer: Timer = $MarginContainer/HideTimer  # Timer for auto-hiding

const MAX_WIDTH = 1000
const DISPLAY_DURATION = 3.0  # Time before hiding the text box

var text = ""
var letter_index = 0

var letter_time = 0.1
var space_time = 0.15
var punctuation_time = 0.2

var is_displaying = false  # Track if text is currently being shown

signal finished_displaying()



# Audio stream player
@onready var audio_stream_player_3d: AudioStreamPlayer3D = $"../../AudioStreamPlayer3D"


# Dictionary of predefined texts
var predefined_texts = {
	"greeting":
		["Hi! I'm Markie, your new input device!", # EN
		"¡Hola! Soy Markie, tu nuevo dispositivo de entrada!", # ES
		"Ciao! Sono Markie, il tuo nuovo dispositivo di input!", # IT
		"Merhaba! Ben Markie, yeni giriş cihazın"], # TR
	"turning_left":
		["Turning the object left!", # EN
		"Girando el objeto a la izquierda!", # ES
		"Ruotando l'oggetto a sinistra!", # IT
		"Nesneyi sola döndürüyorum!"], # TR
	"turning_right":
		["Turning the object right!", # EN
		"Girando el objeto a la derecha!", # ES
		"Ruotando l'oggetto a destra!", # IT
		"Nesneyi sağa döndürüyorum!"], # TR
	"turning_up":
		["Turning the object up!", # EN
		"Rotando el objeto para arriba!", # ES
		"Ruotando l'oggetto verso l'alto!", # IT
		"Nesneyi yukarı döndürüyorum!"], # TR
	"turning_down":
		["Turning the object down!", # EN
		"Rotando el objeto para abajo", # ES
		"Ruotando l'oggetto verso il basso!", # IT
		"Nesneyi aşağı döndürüyorum!"], # TR
	"moving_nearer":
		["Moving the object nearer!", # EN
		"Acercando el objeto!", # ES
		"Avvicinando l'oggetto!", # IT
		"Nesneyi yaklaştırıyorum!"], # TR
	"moving_further":
		["Moving the object further!", # EN
		"Alejando el objeto!", # ES
		"Allontanando l'oggetto!", # IT
		"Nesneyi uzaklaştırıyorum!"], # TR
	"changing_scene":
		["Changing the scene!", # EN
		"Cambiando la escena!", # ES
		"Cambiando la scena!", # IT
		"Sahneyi değiştiriyorum!"], # TR
	"changing_language":
		["Changing the language!", # EN
		"Cambiando el idioma!", # ES
		"Cambiando la lingua!", # IT
		"Dili değiştiriyorum!"], # TR
}
# Dictionary of predefined audios
# EN -> https://huggingface.co/spaces/k2-fsa/text-to-speech
var predefined_audios = {
	"greeting":
		["res://Audios/EN/greetings_EN.mp3", # EN
		"res://Audios/ES/greetings_ES.mp3", # ES
		"res://Audios/IT/greetings_IT.mp3", # IT
		"res://Audios/TR/greetings_TR.mp3"], # TR
	"turning_left":
		["res://Audios/EN/turning_left_EN.mp3", # EN
		"res://Audios/ES/turning_left_ES.mp3", # ES
		"res://Audios/IT/turning_left_IT.mp3", # IT
		"res://Audios/TR/turning_left_TR.mp3"], # TR
	"turning_right":
		["res://Audios/EN/turning_right_EN.mp3", # EN
		"res://Audios/ES/turning_right_ES.mp3", # ES
		"res://Audios/IT/turning_right_IT.mp3", # IT
		"res://Audios/TR/turning_right_TR.mp3"], # TR
	"turning_up":
		["res://Audios/EN/turning_up_EN.mp3", # EN
		"res://Audios/ES/turning_up_ES.mp3", # ES
		"res://Audios/IT/turning_up_IT.mp3", # IT
		"res://Audios/TR/turning_up_TR.mp3"], # TR
	"turning_down":
		["res://Audios/EN/turning_down_EN.mp3", # EN
		"res://Audios/ES/turning_down_ES.mp3", # ES
		"res://Audios/IT/turning_down_IT.mp3", # IT
		"res://Audios/TR/turning_down_TR.mp3"], # TR
	"moving_nearer":
		["res://Audios/EN/moving_nearer_EN.mp3", # EN
		"res://Audios/ES/moving_nearer_ES.mp3", # ES
		"res://Audios/IT/moving_nearer_IT.mp3", # IT
		"res://Audios/TR/moving_nearer_TR.mp3"], # TR
	"moving_further":
		["res://Audios/EN/moving_further_EN.mp3", # EN
		"res://Audios/ES/moving_further_ES.mp3", # ES
		"res://Audios/IT/moving_further_IT.mp3", # IT
		"res://Audios/TR/moving_further_TR.mp3"], # TR
	"changing_scene":
		["res://Audios/EN/changing_scene_EN.mp3", # EN
		"res://Audios/ES/changing_scene_ES.mp3", # ES
		"res://Audios/IT/changing_scene_IT.mp3", # IT
		"res://Audios/TR/changing_scene_TR.mp3"], # TR
	"changing_language":
		["res://Audios/EN/changing_language_EN.mp3", # EN
		"res://Audios/ES/changing_language_ES.mp3", # ES
		"res://Audios/IT/changing_language_IT.mp3", # IT
		"res://Audios/TR/changing_language_TR.mp3"], # TR
}



func _ready():
	visible = false  # Ensure it's hidden by default

func display_text(text_to_display: String):
	if is_displaying:
		return  # Ignore if a message is already being displayed

	# Display text
	is_displaying = true
	text = text_to_display
	label.text = ""
	visible = true  # Show the text box
	letter_index = 0
	_display_letter()
	
	
func display_text_and_audio(text_to_display: String, audio_to_display: String):
	if is_displaying:
		return  # Ignore if a message is already being displayed
	
	# Play the audio
	var audio_stream = load(audio_to_display)
	if audio_stream:
		audio_stream_player_3d.stream = audio_stream
		audio_stream_player_3d.play()
	else:
		print("Error: Unable to load audio file:", audio_to_display)
	
	# Display text
	is_displaying = true
	text = text_to_display
	label.text = ""
	visible = true  # Show the text box
	letter_index = 0
	_display_letter()

func display_predefined_text(keyword: String, language: int):
	if predefined_texts.has(keyword):
		display_text_and_audio(predefined_texts[keyword][language], predefined_audios[keyword][language])  # Use the predefined text
	else:
		print("Error: No predefined text found for key:", keyword)

func _display_letter():
	if letter_index < text.length():
		label.text += text[letter_index]
		letter_index += 1

		match text[letter_index - 1]:  # Previous character determines delay
			"!", ".", ",", "?":
				timer.start(punctuation_time)
			" ":
				timer.start(space_time)
			_:
				timer.start(letter_time)
	else:
		finished_displaying.emit()
		hide_timer.start(DISPLAY_DURATION)  # Start the timer to hide text

func _on_letter_display_timer_timeout():
	_display_letter()

func _on_hide_timer_timeout() -> void:
	visible = false  # Hide the text box
	is_displaying = false  # Reset display status
