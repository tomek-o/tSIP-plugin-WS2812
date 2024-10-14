while (1) do
	PluginSendMessageText("WS2812.dll", "WRITE 0 0 10")
	Sleep(250)
	PluginSendMessageText("WS2812.dll", "WRITE 0 0 0")
	Sleep(250)
	local ret = CheckBreak()
	-- break on user request
	if ret ~= 0 then
		print ('User break\n')
		break
	end
end
