extends Node3D

var process_io: FileAccess
var process: Dictionary
var command: String = ""
var output_thread: Thread

func _ready():
	var executable_path = "/home/paul/Desktop/Markie_Stuff/Markie/QT_FiducialObject/fiducial_object/build/utils_developer/inputDevice"
	var arguments = ["/home/paul/Desktop/Markie_Stuff/Markie/Resources/big_dodec_calib/calib/cameraCalibration.yml", "0.035", "/home/paul/Desktop/Markie_Stuff/Markie/Resources/big_dodec_calib/map.yml", "-detectionMode=2", "-inputVideo=live:0", "-noOutputVideo", "-minimal"]
	process = OS.execute_with_pipe(executable_path, arguments)
	process_io = process["stdio"]
	start_reading_output()

func start_reading_output():
	output_thread = Thread.new()
	var callable = Callable(self, "_read_output")
	output_thread.start(callable)

func _read_output():
	while not process_io.eof_reached():
		var line = process_io.get_line()
		if line:
			command = line
			print("Received from C++: %s" % line)
		OS.delay_msec(5)

func _exit_tree():
	if process and process.has("pid"):
		OS.kill(process["pid"])
	if output_thread and output_thread.is_alive():
		output_thread.wait_to_finish()
