
EXT_X_STREAM_INF	= "#EXT-X-STREAM-INF:"

function parse_m3u8Data(url, parse_mode)
	local box = paintMiniInfoBox(readData);

	os.remove(m3u8Data);
	local cmd = wget_cmd .. m3u8Data .. " '" .. url .. "'";
--	print(cmd);
	os.execute(cmd);

	local streamInfo = {};
	local fp, s;
	fp = io.open(m3u8Data, "r");
	if fp == nil then
		gui.hideInfoBox(box)
		error("Error connecting url.")
	end;
	local count = 1;
	for line in fp:lines() do
		line = helpers.trim(line);
		local found = n:strFind(line, EXT_X_STREAM_INF);
		if (found ~= nil) then
			local a, b, c
			local bandwidth = 0;
			local resolution = "-";
			local codec = {};
			a, b, bandwidth, resolution, c = string.find(line, 'BANDWIDTH=(.*),RESOLUTION=(.*),CODECS=(.*)')
			if (a == nil) then
				bandwidth = 0;
				resolution = "-";
				a, b, bandwidth, c, resolution = string.find(line, 'BANDWIDTH=(.*),CODECS=(.*),RESOLUTION=(.*)')
			end
			if (a == nil) then
				bandwidth = 0;
				resolution = "-";
				a, b, bandwidth = string.find(line, 'BANDWIDTH=(.*)')
			end
			if c ~= nil then
				c = string.gsub(c, "\"", "")
				codec = helpers.split(c, ",")
				local i
				for i = 1, #codec do
					codec[i] = helpers.trim(codec[i])
				end
			end
			if (count > 1) then
				if (streamInfo[count-1]['bandwidth'] == tonumber(bandwidth)) then
					count = count - 1;
				end
			end
			streamInfo[count] = {}
			streamInfo[count]['bandwidth'] = tonumber(bandwidth)
			streamInfo[count]['resolution'] = resolution
			streamInfo[count]['codec'] = {}
			streamInfo[count]['codec'] = codec
			count = count + 1;
		else
			if ((count > 1) and (#line > 2)) then
				-- url
				if (parse_mode == 1) then
					local found = n:strFind(line, "http");
					if (found == nil) then
						found = n:strFind(line, "rtmp");
					end
					if (found == nil or (found ~= nil and found ~= 0)) then
						line = posix.dirname(url) .. "/" .. line
					end
					streamInfo[count-1]['url'] = line
				elseif (parse_mode == 2) then
					streamInfo[count-1]['url'] = url
				end
			end

		end
	end

	fp:close();
	gui.hideInfoBox(box)
	return streamInfo;
end

function get_m3u8url(url, parse_mode)
	local ret = {}
	local si = parse_m3u8Data(url, parse_mode);

	if (#si < 1) then
		ret['url']           = url;
		ret['bandwidth']     = "-";
		ret['resolution']    = "-";
		return ret
	end

	local i
	local minBW    = 1000000000
	local maxBW    = 0
	local tmpBW    = 0
	local xBW      = 0
	local maxRes   = ""
	local minRes   = ""
	local xRes     = ""
	local minUrl   = ""
	local maxUrl   = ""
	local xUrl     = ""

	-- min/max bandwidth
	for i = 1, #si do
		if (si[i]['bandwidth'] == nil) then si[i]['bandwidth'] = 0 end
		if (si[i]['bandwidth'] > 90000) then -- skip audio streams
			if (si[i]['bandwidth'] <= minBW) then
				minBW = si[i]['bandwidth']
				minUrl = si[i]['url']
				minRes = si[i]['resolution']
			end
			if (si[i]['bandwidth'] >= maxBW) then
				maxBW = si[i]['bandwidth']
				maxUrl = si[i]['url']
				maxRes = si[i]['resolution']
			end
		end
	end

	-- average bandwidth
	tmpBW = (maxBW+minBW)/2
	local diff = 1000000000
	for i = 1, #si do
		if (si[i]['bandwidth'] == nil) then si[i]['bandwidth'] = 0 end
		if (si[i]['bandwidth'] > 90000) then -- skip audio streams
			if (math.abs(tmpBW - si[i]['bandwidth']) < diff) then
				diff = math.abs(tmpBW - si[i]['bandwidth'])
				xUrl = si[i]['url']
				xBW = si[i]['bandwidth']
				xRes = si[i]['resolution']
			end
		end
	end

--	helpers.tprint(si)
--	helpers.printf("minBW: %d, maxBW: %d, tmpBW: %d", minBW, maxBW, tmpBW)

	if (conf.streamQuality == "max") then
		-- max
		ret['url']           = maxUrl;
		ret['bandwidth']     = maxBW;
		ret['resolution']    = maxRes;
	elseif (conf.streamQuality == "normal") then
		-- normal
		ret['url']           = xUrl;
		ret['bandwidth']     = xBW;
		ret['resolution']    = xRes;
	else
		-- min
		ret['url']           = minUrl;
		ret['bandwidth']     = minBW;
		ret['resolution']    = minRes;
	end

	return ret
end
