target("platform.kintex7.fatfs-sd")
	set_kind("static")

	add_deps("file.driver.fatfs.backend.sd", "platform.kintex7")
	add_files("src/**.cpp")