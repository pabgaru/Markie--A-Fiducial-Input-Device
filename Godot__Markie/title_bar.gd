extends Control

#@onready var animated_sprite_2d: AnimatedSprite2D = $Markie/Markie_Animations
@onready var animated_sprite_2d: AnimatedSprite2D = $"../AnimatedSprite2D"

# Buttons
@onready var close_button: TextureButton = $"../Buttons/CloseButton"
@onready var amplify_button: TextureButton = $"../Buttons/AmplifyButton"


# Variables to track following state and timing
var following = false
var dragging_start_position = Vector2i()
var following_timer = 0.0
var threshold_toGetNauseus = 5.0


func _gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		if event.get_button_index() == 1:
			following = !following
			if following:
				# Start the timer and drag position when following starts
				following_timer = 0.0
				dragging_start_position = Vector2i(get_local_mouse_position())
				#animated_sprite_2d.set_animations("eye_moving")  # Start moving eyes
				animated_sprite_2d.set_animations("eye_moving")
			else:
				# Stop following and transition to idle or nauseus_to_idle
				animated_sprite_2d.set_animations("nauseus_to_idle" if animated_sprite_2d.isNauseus else "idle")

func _process(delta: float) -> void:
	if following:
		following_timer += delta
		if following_timer > threshold_toGetNauseus and not animated_sprite_2d.isNauseus:
			# Trigger nauseus state after 2 seconds of following
			animated_sprite_2d.set_animations("idle_to_nauseus")
		
		# Move the window following the mouse
		var new_position = get_window().position + Vector2i(get_global_mouse_position()) - dragging_start_position
		get_window().position = new_position


func _on_close_button_pressed() -> void:
	get_tree().quit()


func _on_minimize_button_pressed() -> void:
	DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_MINIMIZED)


var previous_mode = DisplayServer.WINDOW_MODE_WINDOWED  # Store the previous mode
func _on_maximize_button_pressed() -> void:
	var current_mode = DisplayServer.window_get_mode()  # Get current window mode
	if current_mode == DisplayServer.WINDOW_MODE_MAXIMIZED:
		DisplayServer.window_set_mode(previous_mode)  # Restore previous mode
	else:
		previous_mode = current_mode  # Store current mode before maximizing
		DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_MAXIMIZED)
