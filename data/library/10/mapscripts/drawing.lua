-- OctaForge drawing application, allows drawing in 3D world.
-- left mouse button draws
-- middle changes color
-- right ends current line/curve
-- built on OF API v1
-- author: q66 <quaker66@gmail.com>

-- set up "shoot mode" variable aloas, which will be persistent.
-- you can then toggle shooting and drawing from the console using this.
-- because it's persistent, your last state (drawing / shooting) will
-- be saved and applied the next run.
if not EVAR.shoot_mode then
    local was_persisting = var.persist_vars(true)
    var.new("shoot_mode", EAPI.VAR_I, 0)
    var.persist_vars(was_persisting)
end

-- Register our custom player entity class into storage
entity_classes.register(plugins.bake(
    character.player, {
        game_manager.player_plugin,
-- enable for platformer game
--      platformer.plugin,
        firing.plugins.protocol,
        firing.plugins.player,
        health.plugin,
        projectiles.plugin,
        chaingun.chaingun.plugin,
        {
            properties = {
                new_mark = state_variables.state_array_float({ client_set = true, has_history = false })
            },

            -- player gun indexes and current gun
            init = function(self)
                self.gun_indexes = { player_chaingun, player_rocket_launcher }
                self.current_gun_index = player_chaingun
            end,

            -- Switches color in entity
            next_color = function(self)
                local len = #self.colors
                if  self.color_id < len then
                    self.color_id = self.color_id + 1
                else
                    self.color_id = 1
                end
                self.color = self.colors[self.color_id]
            end,

            -- Creates a "dummy" mark to stop current line
            reset_mark = function(self)
                self.new_mark   = { -1, -1, -1 }
                self.stop_batch = true
            end,

            -- This is called when new mark is created. It adds a point into a storage.
            -- vec4's are used, first three elements for position, fourth for color.
            on_new_mark = function(self, mark)
                if #mark == 3 then
                    mark = math.Vec4(mark[1], mark[2], mark[3], self.color)
                else
                    mark = nil
                end
                table.insert(self.marks, mark)
            end,

            -- Called right after initialization on client
            client_activate = function(self, kwargs)
                -- Mark storage
                self.marks    = {}

                -- Available colors
                self.colors   = {
                    0xFFFFFF, 0xFF0000,
                    0x00FF00, 0x0000FF,
                    0xFFFF00, 0xFF00FF,
                    0x00FFFF
                }
                -- Current color index
                self.color_id = 1
                -- Current color
                self.color    = self.colors[1]

                -- When new_mark state variable is modified, let's call on_new_mark.
                signal.connect(self,state_variables.get_on_modify_name("new_mark"), self.on_new_mark)
            end,

            -- Called every frame on client after initialization
            client_act = function(self, sec)
                -- Draw all marks.
                local last = nil

                for i, mark in pairs(self.marks) do
                    if last and mark and mark.x >= 0 and last.x >= 0 then
                        effects.flare(effects.PARTICLE.STREAK, last, mark, 0, mark.w, 1.0)
                        effects.flare(effects.PARTICLE.STREAK, mark, last, 0, mark.w, 1.0)
                    end
                    last = mark
                end

                -- Check if to draw new batch, or continue
                local newbatch = #self.marks == 0 or not self.marks[#self.marks - 1]
                local conbatch = #self.marks >  0 and    self.marks[#self.marks - 1]

                -- If continuing and haven't just stopped, draw a spark at the end of last mark.
                if conbatch and not self.stop_batch then
                    effects.splash(
                        effects.PARTICLE.SPARK, 10, 0.15,
                        self.marks[#self.marks - 1],
                        self.marks[#self.marks - 1].w,
                        1.0, 25, 1
                    );
                end

                -- If we're pressing left mouse button, let's draw new stuff
                if self.pressing then
                    local newpos = input.get_target_position()
                    local toplyr = self.position:sub_new(newpos)
                    newpos:add(toplyr:normalize():mul(1.0)) -- bring a little out of the scenery
                    if newbatch or not self.marks[#self.marks - 1]:is_close_to(newpos, 5.0) then
                        self.new_mark = newpos:to_array()
                    end
                end
            end
        }
    }, "game_player"
), "fpsent")

-- set up a chaingun (non-projectile, repeating)
player_chaingun        = firing.register_gun(
    chaingun.chaingun(), "chaingun"
)
-- and a rocket launcher (projectile, non-repeating)
player_rocket_launcher = firing.register_gun(
    rocket_launcher.rocket_launcher(), "rocket_launcher"
)

-- Override clientside click method.
-- When left mouse button is clicked, set pressing to down, and disable stop_batch.
-- When middle mouse button is clicked, change to next color.
-- When right mouse button is clicked, stop drawing current batch and go to new one.
function client_click(btn, down, pos, ent, x, y)
    if  ent and ent.client_click then
        ent:client_click(btn, down, pos, x, y)
    end

    -- in shoot mode, shoot instead of drawing
    if EVAR.shoot_mode == 1 then
        return firing.client_click(btn, down, pos, ent, x, y)
    end

    if btn == 1 then
        entity_store.get_player_entity().pressing   = down
        entity_store.get_player_entity().stop_batch = false
    elseif btn == 2 and down then
        entity_store.get_player_entity():reset_mark()
    elseif btn == 3 and down then
        entity_store.get_player_entity():next_color()
    end
end

-- enable for platformer game
-- do_movement  = platformer.do_movement
-- do_strafe    = platformer.do_strafe
-- do_mousemove = platformer.do_mousemove

-- Notify the engine that we're overriding player by setting engine variable
EVAR.player_class = "game_player"

-- This way you can disable gravity, not needed, default value is 200
-- world.gravity = 0
