function main(arch_string)
	local arch_match = arch_string:match("rv%d%d(.*)")

	if arch_match == nil then
		return nil
	end

	local split_result = arch_match:split("_")

	local single_letters = {}

	for letter in split_result[1]:gmatch(".") do
		table.insert(single_letters, letter)
	end

	local concated_table = (table.concat(single_letters, "_") .. "_" .. table.concat(split_result, "_", 2)):split("_")

	-- Atomic

	if table.contains(concated_table, "a") and not table.contains(concated_table, "zaamo") then
		table.insert(concated_table, "zaamo")
	end
	if table.contains(concated_table, "a") and not table.contains(concated_table, "zalrsc") then
		table.insert(concated_table, "zalrsc")
	end
	if table.contains(concated_table, "zaamo") and table.contains(concated_table, "zalrsc") and not table.contains(concated_table, "a") then
		table.insert(concated_table, "a")
	end

	table.sort(concated_table)
	return concated_table
end