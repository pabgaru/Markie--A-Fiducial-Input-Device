extends AnimatedSprite2D

@export var isNauseus: bool = false  # Expose nauseus state for external scripts
@onready var timer: Timer = Timer.new()  # Timer for automatic behaviors

func _ready() -> void:
	add_child(timer)
	timer.wait_time = 3.0
	timer.connect("timeout", Callable(self, "_on_timer_timeout"))
	connect("animation_finished", Callable(self, "_on_animation_finished"))
	set_animations("idle")  # Start with idle animation

func set_animations(animation_name: String) -> void:
	# Handle animation logic based on input
	match animation_name:
		"idle":
			isNauseus = false
			play("idle")
			if timer.is_stopped():
				timer.start()
		"parpadeo":
			isNauseus = false
			play("parpadeo")
			timer.stop()  # Stop timer while "parpadeo" is playing
		"eye_moving":
			timer.stop()
			play("eye_moving")
		"idle_to_nauseus":
			isNauseus = true
			timer.stop()
			play("idle_to_nauseus")
		"nauseus_to_idle":
			isNauseus = false
			timer.stop()
			play("nauseus_to_idle")
		"nauseus":
			isNauseus = true
			timer.stop()
			play("nauseus")
		_:
			print("Unknown animation:", animation_name)

func _on_timer_timeout() -> void:
	# Play "parpadeo" only if the current animation is "idle"
	if animation == "idle":
		set_animations("parpadeo")

func _on_animation_finished() -> void:
	if animation == "parpadeo":
		set_animations("idle")
	elif animation == "nauseus_to_idle":
		set_animations("idle")
	elif animation == "idle_to_nauseus":
		set_animations("nauseus")
	elif animation == "eye_looks_down":
		set_animations("idle")
	elif animation == "eye_looks_left":
		set_animations("idle")
	elif animation == "eye_looks_right":
		set_animations("idle")
	elif animation == "eye_looks_up":
		set_animations("idle")
