#!/usr/bin/env lua

local ok, map = pcall(dofile, arg[1] or "map.lua")
if not ok then
	error("Can't open input file!")
end

local tileids = {}
local tiles_x = map.tilesets[1].imagewidth / map.tilesets[1].tilewidth
local tiles_y = map.tilesets[1].imageheight / map.tilesets[1].tileheight

local counter = 0
local trans_ids = {}
for x=0, tiles_x-1 do
	local ccolumn = {}
	for y=0, tiles_y-1 do
		ccolumn[y] = counter
		counter = counter + 1
	end
	trans_ids[x] = ccolumn
end

function convertTileID(tileid)
	local tileid = tileid - 1
	local x = tileid % tiles_x
	local y = (tileid - x) / tiles_x
	return trans_ids[x][y]
end

local lines = {}
local cline = {}
for k,v in pairs(map.layers[1].data) do
	local text = string.format("%x", convertTileID(v))
	if #text < 2 then
		text = "0" .. text
	end
	text = "0x" .. text
	table.insert(cline, text)
	if #cline >= map.width then
		table.insert(lines, cline)
		cline = {}
	end
end

local columns = {}
for i=1, map.width do
	local ccolumn = {}
	for _, line in ipairs(lines) do
		table.insert(ccolumn, line[i])
	end
	table.insert(columns, ccolumn)
end

local ret = "{ "
for _,column in ipairs(columns) do
	ret = ret .. table.concat(column, ", ") .. "\n"
end
ret = ret:sub(1,-2) .. " };"

local outfile = io.open(arg[2] or "map.c", "w")
outfile:write(ret)
