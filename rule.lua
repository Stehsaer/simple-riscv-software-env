rule("generate.map")

	on_load(function(target)
		target:add("ldflags", "-Map " .. target:targetfile() .. ".map", {force=true})
	end)

rule_end()