CharacterController = ScriptObject()

function CharacterController:Start()
	self:SubscribeToEvent("KeyDown", "CharacterController:HandleKeyDown")
end

function CharacterController:HandleKeyDown(eventType, eventData)
	local key = eventData["Key"]:GetInt()
	if key == KEY_W then
	end
end
