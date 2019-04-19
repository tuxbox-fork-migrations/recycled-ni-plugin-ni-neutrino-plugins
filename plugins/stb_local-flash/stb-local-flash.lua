-- The Tuxbox Copyright
--
-- Copyright 2018 The Tuxbox Project. All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without modification, 
-- are permitted provided that the following conditions are met:
--
-- Redistributions of source code must retain the above copyright notice, this list
-- of conditions and the following disclaimer. Redistributions in binary form must
-- reproduce the above copyright notice, this list of conditions and the following
-- disclaimer in the documentation and/or other materials provided with the distribution.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS`` AND ANY EXPRESS OR IMPLIED
-- WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
-- HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
-- EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
-- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
-- HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
-- OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
-- SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--
-- The views and conclusions contained in the software and documentation are those of the
-- authors and should not be interpreted as representing official policies, either expressed
-- or implied, of the Tuxbox Project.

caption = "STB-Local-Flash"

local posix = require "posix"
n = neutrino()
fh = filehelpers.new()

bootfile = "/boot/STARTUP"
devbase = "linuxrootfs"

local g = {}
locale = {}

locale["deutsch"] = {
	current_boot_partition = "Die aktuelle Startpartition ist: ",
	choose_partition = "\n\nBitte wählen Sie die Flash-Partition aus",
	start_partition1 = "Lokales Image in die gewählte Partition ",
	start_partition2 = " flashen?",
	flash_partition1 = "Image wird in die Partition ",
	flash_partition2 = "Daten werden gesichert \n\nBitte warten...",
	flash_partition3 = " geflasht \n\nBitte warten...",
	select_imagepath = "Wählen Sie den Quellpfad aus",
	flash_partition5 = "Flash erfolgreich",
	flash_partition8 = "Entpacken des Images fehlgeschlagen",
	flash_partition9 = "Flashen des Kernel fehlgeschlagen",
	flash_partition10 = "Flashen des Rootfs fehlgeschlagen",
	flash_partition11 = "Partitionsschema ungültig",
	prepare_system = "System wird vorbereitet ... Bitte warten",

}

locale["english"] = {
	current_boot_partition = "The current start partition is: ",
	choose_partition = "\n\nPlease choose the new flash partition",
	start_partition1 = "Flash the selected image into partition ",
	start_partition2 = "?",
	flash_partition1 = "Image will be flashed into partition ",
	flash_partition2 = "Data will be saved \n\nPlease wait...",
	flash_partition3 = " \n\nPlease wait...",
	select_imagepath = "Please select source path",
	flash_partition5 = "Flash succeeded",
	flash_partition8 = "Unpacking the image failed",
	flash_partition9 = "Writing the kernel failed",
	flash_partition10 = "Writing the rootfs failed",
	flash_partition11 = "Partitionscheme invalid",
	prepare_system = "System is getting prepared ... please stand by",
}

function create_servicefile()
	f = io.open("/tmp/flash@.service", "w")
	f:write("[Unit]", "\n")
	f:write("Description=flash on partition %I", "\n")
	f:write("", "\n")
	f:write("[Service]", "\n")
	f:write("ExecStart=/usr/bin/flash %I " .. image_path, "\n")
	f:write("ExecStartPost=/bin/echo -e '\033[?17;0;0c'", "\n")
	f:write("Type=oneshot", "\n")
	f:write("RemainAfterExit=no", "\n")
	f:write("StandardOutput=tty", "\n")
	f:write("TTYPath=/dev/tty1", "\n")
	f:close()
end

function sleep(n)
	os.execute("sleep " .. tonumber(n))
end

for line in io.lines(bootfile) do
	i, j = string.find(line, devbase)
	current_root = tonumber(string.sub(line,j+1,j+1))
end

neutrino_conf = configfile.new()
neutrino_conf:loadConfig("/etc/neutrino/config/neutrino.conf")
lang = neutrino_conf:getString("language", "english")

if locale[lang] == nil then
	lang = "english"
end

function basename(str)
	local name = string.gsub(str, "(.*/)(.*)", "%2")
	return name
end

function get_imagename(root)
	local glob = require "posix".glob
	for _, j in pairs(glob('/boot/*', 0)) do
		for line in io.lines(j) do
			if (j ~= bootfile) then
				if line:match(devbase .. root) then
					imagename = basename(j)
				end
			end
		end
	end
	return imagename
end

timing_menu = neutrino_conf:getString("timing.menu", "0")

chooser_dx = n:scale2Res(600)
chooser_dy = n:scale2Res(200)
chooser_x = SCREEN.OFF_X + (((SCREEN.END_X - SCREEN.OFF_X) - chooser_dx) / 2)
chooser_y = SCREEN.OFF_Y + (((SCREEN.END_Y - SCREEN.OFF_Y) - chooser_dy) / 2)

chooser = cwindow.new {
	x = chooser_x,
	y = chooser_y,
	dx = chooser_dx,
	dy = chooser_dy,
	title = caption,
	icon = "settings",
	has_shadow = true,
	btnRed = get_imagename(1),
	btnGreen = get_imagename(2),
	btnYellow = get_imagename(3),
	btnBlue = get_imagename(4)
}

chooser_text = ctext.new {
	parent = chooser,
	x = OFFSET.INNER_MID,
	y = OFFSET.INNER_SMALL,
	dx = chooser_dx - 2*OFFSET.INNER_MID,
	dy = chooser_dy - chooser:headerHeight() - chooser:footerHeight() - 2*OFFSET.INNER_SMALL,
	text = locale[lang].current_boot_partition .. get_imagename(current_root) .. locale[lang].choose_partition,
	font_text = FONT.MENU,
	mode = "ALIGN_CENTER"
}


function flash_image()

	chooser:paint()

	i = 0
	d = 500 -- ms
	t = (timing_menu * 1000) / d

	if t == 0 then
		t = -1 -- no timeout
	end

	colorkey = nil
	repeat
	i = i + 1
	msg, data = n:GetInput(d)

	if (msg == RC['red']) then
		root = 1
		colorkey = true
	elseif (msg == RC['green']) then
		root = 2
		colorkey = true
	elseif (msg == RC['yellow']) then
		root = 3
		colorkey = true
	elseif (msg == RC['blue']) then
		root = 4
		colorkey = true
	end

	until msg == RC['home'] or colorkey or i == t

	chooser:hide()

	if colorkey then
		res = messagebox.exec {
		title = caption,
		icon = "settings",
		text = locale[lang].start_partition1 .. get_imagename(root) .. locale[lang].start_partition2,
		timeout = 0,
		buttons={ "yes", "no" }
		}
		if res == "yes" then
			if (root == current_root) then
				local file = assert(io.popen("etckeeper commit -a", 'r'))
			end
			create_servicefile()
			os.execute("mv -f /tmp/flash@.service /lib/systemd/system/local_flash@.service")
			local file = assert(io.popen("systemctl start local_flash@" .. root, 'r'))
			return
		end
	end
end




function set_path(id, value)
	image_path=value
	print(image_path)
	return image_path
end

function main_menu()
	g.main = menu.new{name=caption, icon="settings"}
	m=g.main
	m:addItem{type="back"}
	m:addItem{type="separatorline"}
	m:addItem{type="filebrowser", dir_mode="1", id=image_path, name=locale[lang].select_imagepath, action="set_path",
		enabled=true, icon="rot", directkey=RC["red"],  hint_icon="hint_service"
		 };
	m:addItem{type="forwarder", name="Flash", action="flash_image", icon="gruen", directkey=RC["green"]};
	m:exec()
	m:hide()
end

main_menu()
