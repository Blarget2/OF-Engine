-- Example script which draws a sine "flower" in the world procedurally.
-- built on OF API v1
-- author: q66 <quaker66@gmail.com>

-- Create a custom player class
myplayer = class.new(character.player, nil, "myplayer")

-- Called right after initialization on client
function myplayer:client_activate(kwargs)
    self.base_class.client_activate(self, kwargs)
    -- Current rotation angle
    self.angle = 1
    -- Current circle radius
    self.circle_rad = 1
    -- Old angle for 360 checks
    self.old_angle = 1
end

-- Called every frame on client after initialization
function myplayer:client_act(sec)
    self.base_class.client_act(self, sec)
    -- Loop just until radius reaches 60, so it doesn't take too long.
    -- Make bigger for bigger circles
    if self.circle_rad <= 60 then
        -- The X position. Cosine calculation
        self.position.x = (math.cos(math.rad(self.angle)) * self.circle_rad) + 150
        -- The Y position. Sine calculation
        self.position.y = (math.sin(math.rad(self.angle)) * self.circle_rad) + 150
        -- Height. Basically makes a sine curve so the player goes up and down with the right period
        self.position.z = math.sin(math.rad(self.angle) * 10) * 2 + 650

        -- Create cube on current position. Use minimal size
        edit.procedural.create_cube(self.position.x, self.position.y, self.position.z, 1)
    else
        -- If we're over 20, just return from the function after running parent.
        return nil
    end

    -- Increment the angle by 1
    self.angle = self.angle + 1
    -- If we reached another 360 on angle - check done using old_angle
    -- - let's increase radius and make next circle
    if (self.angle - self.old_angle) >= 360 then
        -- Increase radius
        self.circle_rad = self.circle_rad + 1
        -- Update old angle
        self.old_angle = self.angle
    end
end

-- Register our custom player entity class into storage
entity_classes.register(myplayer, "fpsent")

-- Notify the engine that we're overriding player by setting engine variable
EVAR.player_class = "myplayer"

-- This way you can disable gravity, not needed, default value is 200
-- world.gravity = 0
