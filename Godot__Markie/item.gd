extends Node3D

# Commands!
@onready var sub_process: Node3D = $"../subProcess"

# General stuff
@onready var cursor: Sprite2D = $"../Screen Layout/Cursor"
var file_path = "/home/paul/commands.txt"  # Adjust to your file path

# Markie
@onready var animated_sprite_2d: AnimatedSprite2D = $"../Screen Layout/AnimatedSprite2D"

# White border
@onready var white_border: Sprite2D = $"../Screen Layout/WhiteBorder"
# Define the show_white_border_animation function
var white_border_tween: Tween = null
var white_border_duration = 1.5  # Duration the white border stays visible (in seconds)

# Camera
@onready var camera_3d: Camera3D = $"../Camera3D"
# Camera animation settings
var transition_duration = 1.5  # Duration for fade-out/in animation
var animation_in_progress = false  # Prevent multiple animations


# TextBox
@onready var text_box: MarginContainer = $"../Screen Layout/TextBox"


# Language
var lang = 0 # EN as default language
# Available langs
# 0 - EN
# 1 - ES
# 2 - IT
# 3 - TR
@onready var cover: Sprite2D = $"../Screen Layout/Cover"
@onready var langtag: Sprite2D = $"../Screen Layout/Langtag"
# The cover needs to be set with the texture path of every language
var cover_paths = {
	0: "res://Covers/cover_EN.png",
	1: "res://Covers/cover_ES.png",
	2: "res://Covers/cover_IT.png",
	3: "res://Covers/cover_TR.png"
}
# The lang tags
var langtag_paths = {
	0: "res://LangTags/langtag_EN.png",
	1: "res://LangTags/langtag_ES.png",
	2: "res://LangTags/langtag_IT.png",
	3: "res://LangTags/langtag_TR.png",
}
# Timer
var can_change_language = true  # Flag to control language change cooldown
var language_timer: Timer  # Timer for language change delay
func _on_language_timer_timeout():
	can_change_language = true  # Reset cooldown flag
# change language function
func changeLanguage():
	if not can_change_language:
		print("Language change is on cooldown!")
		return  # Stop execution if cooldown is active
	can_change_language = false  # Disable further calls
	language_timer.start()  # Start the cooldown timer
	# Increment language ID
	lang += 1
	if lang == 4:
		lang = 0  # Loop back to English if exceeding available languages
	# Update cover language
	if cover_paths.has(lang):
		cover.texture = load(cover_paths[lang])  # Load new cover texture
	# Update langtag language
	if langtag_paths.has(lang):
		langtag.texture = load(langtag_paths[lang])  # Load new cover texture
	# Notify user of language change
	text_box.display_predefined_text("changing_language", lang)
	# White border attention
	show_white_border_animation()

# World environment
@onready var world_environment: WorldEnvironment = $"../WorldEnvironment"

# Sccenes defined
@onready var castle: Node3D = $castle
@onready var statue: Node3D = $statue
@onready var heart: Node3D = $heart
@onready var backpack: Node3D = $backpack

# List of scenes and corresponding colors
var scene_items = []
var current_index = 0  # Index to track the active scene

# Change scene timer
var scene_change_timer: Timer  # Timer for scene change delay
var can_change_scene = true  # Control flag
func _on_scene_change_timer_timeout():
	can_change_scene = true


# nextScene
func nextScene():
	if not can_change_scene or animation_in_progress:
		print("Scene change is on cooldown or animation is in progress!")
		return  # Prevent immediate scene switching
	# White border attention
	show_white_border_animation()
	# Left_message
	text_box.display_predefined_text("changing_scene", lang)
	# Start the transition effect (fade-out)
	animation_in_progress = true
	var tween = create_tween()
	tween.tween_property(camera_3d, "fov", 20.0, transition_duration).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN)
	await tween.finished  # Wait for animation to complete
	# Hide the current scene
	scene_items[current_index]["node"].visible = false
	# Increase index and loop back if necessary
	current_index = (current_index + 1) % scene_items.size()
	# Show the new scene
	update_scene()
	# Wait a little before fading back in
	await get_tree().create_timer(0.5).timeout
	# Start the fade-in transition
	tween = create_tween()
	tween.tween_property(camera_3d, "fov", 75.0, transition_duration).set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
	await tween.finished  # Wait for animation to complete
	# Reset animation flag
	animation_in_progress = false
	# Start cooldown (disable further changes)
	can_change_scene = false
	scene_change_timer.start()


# update_scene
func update_scene():
	# Set the new scene visible
	scene_items[current_index]["node"].visible = true
	# Update background color
	world_environment.environment.background_color = scene_items[current_index]["color"]



# Define the movement speed and rotation speed
var movement_speed = 2.0  # Adjust this value to get the desired movement speed
var rotation_speed = 5.0

# Momentum parameters
var position_velocity = Vector3.ZERO  # Current velocity for position
var rotation_velocity = Vector3.ZERO  # Current velocity for rotation
var position_damping = 0.03  # Adjust to control momentum for position
var rotation_damping = 0.03  # Adjust to control momentum for rotation

# Target velocities
var target_position_velocity = Vector3.ZERO
var target_rotation_velocity = Vector3.ZERO

# Track the last executed command ID
var last_executed_id = -1

# Target position for the cursor
var target_cursor_position: Vector2 = Vector2.ZERO
var cursor_lerp_speed: float = 0.1  # Adjust this value for desired smoothness (0 to 1)


# Called just once! (at the start)
func _ready():
	# Ensure all nodes exist before adding them
	if castle and statue and heart and backpack:
		scene_items = [
			{"node": castle, "color": Color.from_string("#0d0624", Color.BLACK)},  # Dark blueish
			{"node": statue, "color": Color.from_string("#113925", Color.BLACK)},  # Greenish
			{"node": heart, "color": Color.from_string("#391114", Color.BLACK)},   # Reddish
			{"node": backpack, "color": Color.from_string("#053940", Color.BLACK)} # Teal
		]
	else:
		print("Error: One or more scene nodes are missing!")
		return  # Stop execution if nodes are missing
	
	# Hide all scenes except the first one
	for i in range(scene_items.size()):
		scene_items[i]["node"].visible = (i == 0)  # Only first scene is visible
	
	# Set the initial background color
	world_environment.environment.background_color = scene_items[0]["color"]
	
	# Initialize and start the scene change timer
	scene_change_timer = Timer.new()
	add_child(scene_change_timer)
	scene_change_timer.wait_time = 5.0  # 5 seconds delay
	scene_change_timer.one_shot = true
	scene_change_timer.connect("timeout", _on_scene_change_timer_timeout)
	
	# Initialize the tween and hide the white border by default
	white_border_tween = Tween.new()
	# Add the tween to the current Node3D, or any appropriate parent node in the scene
	#add_child(white_border_tween)
	white_border.visible = false  # Start with the white border hidden
	
	# By default the language is EN
	# So we do nothing about it
	# Create a Timer for language change delay
	language_timer = Timer.new()
	add_child(language_timer)
	language_timer.wait_time = 2.0  # 2 seconds delay
	language_timer.one_shot = true  # Ensures it only triggers once per call
	language_timer.connect("timeout", _on_language_timer_timeout)


var initialMessageDone = false
# Function to show the white border with an animation
func show_white_border_animation() -> void:
	if animation_in_progress:
		return  # Don't proceed if an animation is already in progress

	animation_in_progress = true
	white_border.visible = true  # Ensure the white border is visible
	white_border.modulate.a = 0  # Start with the border completely transparent

	# Create a new Tween instance for the fade-in animation
	var fade_in_tween: Tween = create_tween()
	fade_in_tween.tween_property(white_border, "modulate:a", 1.0, transition_duration).set_trans(Tween.TRANS_LINEAR).set_ease(Tween.EASE_IN)
	await fade_in_tween.finished  # Wait for the fade-in to complete

	# Wait for the border to stay visible for the specified duration
	await get_tree().create_timer(white_border_duration).timeout

	# Create a new Tween instance for the fade-out animation
	var fade_out_tween: Tween = create_tween()
	fade_out_tween.tween_property(white_border, "modulate:a", 0.0, transition_duration).set_trans(Tween.TRANS_LINEAR).set_ease(Tween.EASE_OUT)
	await fade_out_tween.finished  # Wait for the fade-out to complete

	white_border.visible = false  # Hide the white border after fading out
	animation_in_progress = false  # Reset the animation flag






# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	var autoFile = true
	if autoFile:
		var command_line = read_last_line(file_path)
		# By file
		if command_line != "":
			var parsed_command = parse_command_line(command_line)
			if parsed_command.has("valid") and parsed_command["valid"]:
				execute_commands_with_data(parsed_command, delta)
		# By std::cout
		#var command_line2 = sub_process.command
		#if command_line2 != "":
		#	var parsed_command = parse_command_line(command_line2)
		#	if parsed_command.has("valid") and parsed_command["valid"]:
		#		execute_commands_with_data(parsed_command, delta)

	# Initial message
	if(!initialMessageDone):
		# Left_message
		text_box.display_predefined_text("greeting", lang)
		initialMessageDone = true
	
	# Apply momentum to position and rotation
	position_velocity = position_velocity.lerp(target_position_velocity, position_damping)
	rotation_velocity = rotation_velocity.lerp(target_rotation_velocity, rotation_damping)

	# Apply movement and rotation
	transform.origin += position_velocity * delta
	apply_rotation(rotation_velocity * delta)

	# Reset target velocities for the next frame
	target_position_velocity = Vector3.ZERO
	target_rotation_velocity = Vector3.ZERO
	
	# Lerp the cursor's position toward the target position
	cursor.position = cursor.position.lerp(target_cursor_position, cursor_lerp_speed)
	

func read_last_line(path: String) -> String:
	var file = FileAccess.open(path, FileAccess.READ)
	if file:
		var lines = file.get_as_text().split("\n")
		file.close()
		return lines[-2] if len(lines) > 1 else ""  # Return last valid line
	return ""

func parse_command_line(command_line: String) -> Dictionary:
	# Initialize the result dictionary
	var result = { "valid": false }

	# Split the line into coordinates, command ID, and commands
	var parts = command_line.split("|")
	if parts.size() < 2:
		print("Invalid command format:", command_line)
		return result

	var coords_part = parts[0]
	var command_id_and_commands = parts[1].split(":")
	if command_id_and_commands.size() < 2:
		print("Invalid command format:", command_line)
		return result

	# Parse coordinates
	var coords = coords_part.split(",")
	if coords.size() < 2:
		print("Invalid coordinates format:", coords_part)
		return result

	var x = coords[0].to_float()
	var y = coords[1].to_float()

	# Parse command ID
	var command_id = command_id_and_commands[0].strip_edges().to_int()

	# Parse commands
	var commands = command_id_and_commands[1].strip_edges().split(" ")

	# Set valid flag and return the parsed data
	result["valid"] = true
	result["x"] = x
	result["y"] = y
	result["command_id"] = command_id
	result["commands"] = commands
	return result

func execute_commands_with_data(data: Dictionary, delta: float) -> void:
	
	# Ignore commands with IDs lower than the last executed one
	if data["command_id"] <= last_executed_id:
		print("Skipping command with ID:", data["command_id"])
		return

	# Update the last executed ID
	last_executed_id = data["command_id"]
	
	# Move the cursor to the position specified in the command
	move_cursor_to_position(data["x"], data["y"])

	# Log coordinates
	print("Executing commands at position: (", data["x"], ", ", data["y"], ") with ID:", data["command_id"])

	# Execute commands
	for command in data["commands"]:
		execute_command(command.strip_edges(), delta)


func execute_command(command: String, delta: float) -> void:
	match command:
		"up":
			# Rotate around the camera's global X-axis (pitch)
			var camera_x_axis = camera_3d.global_transform.basis.x
			target_rotation_velocity += camera_x_axis * -rotation_speed
			# Left_message
			text_box.display_predefined_text("turning_up", lang)
			# White border attention
			show_white_border_animation()
			# Markie looks down
			animated_sprite_2d.play("eyes_looks_up")
		"down":
			# Rotate around the camera's global X-axis (pitch)
			var camera_x_axis = camera_3d.global_transform.basis.x
			target_rotation_velocity += camera_x_axis * rotation_speed
			# Left_message
			text_box.display_predefined_text("turning_down", lang)
			# White border attention
			show_white_border_animation()
			# Markie looks down
			animated_sprite_2d.play("eyes_looks_down")
		"left":
			# Rotate around the camera's global Y-axis (yaw)
			var camera_y_axis = camera_3d.global_transform.basis.y
			target_rotation_velocity += camera_y_axis * rotation_speed
			# Left_message
			text_box.display_predefined_text("turning_right", lang)
			# White border attention
			show_white_border_animation()
			# Markie looks down
			animated_sprite_2d.play("eyes_looks_right")
		"right":
			# Rotate around the camera's global Y-axis (yaw)
			var camera_y_axis = camera_3d.global_transform.basis.y
			target_rotation_velocity += camera_y_axis * -rotation_speed
			# Left_message
			text_box.display_predefined_text("turning_left", lang)
			# White border attention
			show_white_border_animation()
			# Markie looks down
			animated_sprite_2d.play("eyes_looks_left")
		"move_nearer":
			# Move along the camera's Z-axis (backward)
			var move_direction = camera_3d.global_transform.basis.z.normalized()
			target_position_velocity += move_direction * movement_speed*2
			# Left_message
			text_box.display_predefined_text("moving_nearer", lang)
			# White border attention
			show_white_border_animation()
		"move_further":
			# Move along the camera's Z-axis (forward)
			var move_direction = -camera_3d.global_transform.basis.z.normalized()
			target_position_velocity += move_direction * movement_speed*2
			# Left_message
			text_box.display_predefined_text("moving_further", lang)
			# White border attention
			show_white_border_animation()
		"change_escene":
			# Change the scene
			nextScene()
		"change_language":
			# Change the language
			changeLanguage()
		"none":
			pass  # Do nothing for "none"
		_:
			print("Unknown command:", command)

func apply_rotation(rotation_velocity: Vector3) -> void:
	# Apply momentum to rotation using global axes
	var camera_x_axis = camera_3d.global_transform.basis.x
	var camera_y_axis = camera_3d.global_transform.basis.y
	var camera_z_axis = camera_3d.global_transform.basis.z

	# Apply rotation along the camera's global axes
	var rotation_matrix = Basis(camera_x_axis, rotation_velocity.x) * Basis(camera_y_axis, rotation_velocity.y) * Basis(camera_z_axis, rotation_velocity.z)
	transform.basis = rotation_matrix * transform.basis


func move_cursor_to_position(x: float, y: float) -> void:
	# Get the viewport size
	var viewport_size = get_viewport().get_visible_rect().size
	# Flip the x value horizontally
	x = 1920 - x
	# Rescale x and y to fit the viewport resolution
	var scaled_x = x * viewport_size.x / 1920
	var scaled_y = y * viewport_size.y / 1080
	# Update the target position
	target_cursor_position = Vector2(scaled_x, scaled_y)
