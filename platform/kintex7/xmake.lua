target("platform.kintex7")
	set_kind("static")
	set_languages("c++23", {public=true})

	add_files("src/*.cpp")
	add_deps("platform.common-headers", {public=true})
	add_includedirs("include", {public=true})
target_end()

rule("generate.coe", function()
	add_deps("generate.bin", {order=true})

	after_link(function(target, opt)
		import("core.project.config")
		import("core.project.depend")
        import("utils.progress")
		
        depend.on_changed(function()
			os.run("python3 $(scriptdir)/bin-to-coe.py %s.bin %s.coe 4", target:targetfile(), target:targetfile())
            progress.show(opt.progress, "${bright blue}generating.$(mode) %s", target:targetfile() .. ".coe")
        end, {files = {target:targetfile() .. ".bin"}, changed = target:is_rebuilt()})
	end)

	on_clean(function(target)
		os.rm(target:targetfile() .. ".coe")
	end)
end)

rule("platform.kintex7.config", function()
	on_load(function(target)
		target:add("ldflags", "-L$(projectdir)/platform/kintex7")
	end)
end)
	

