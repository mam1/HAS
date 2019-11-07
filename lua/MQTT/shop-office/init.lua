
function firmwareInfo()
  if (1/3) > 0 then
    print('FLOAT firmware version')
  else
    print('INTEGER firmware version')
  end
end

-- error handler
uart.on(2, "error",
  function(data)
    print("error from uart:", data)
  end)

function abortInit()
    -- initailize abort boolean flag
    abort = false
    print('Press ENTER to abort startup')
    -- if <CR> is pressed, call abortTest
    uart.on("data", "\r", abortTest, 0)

    -- start timer to execute startup function in 5 seconds
    tmr.create():alarm(5000, tmr.ALARM_SINGLE, startup)
    end
    
function abortTest(data)
    -- user requested abort
    abort = true
    -- turns off uart scanning
     uart.on("data") -- unregister callback function
end

function startup()
    uart.on("data") -- unregister callback function
     -- if user requested abort, exit
    if abort == true then
        print('startup aborted')
        return
        end
    -- otherwise, start up
    print('run startup')
    dofile('startup.lua')
end

print('\ninit.lua 0.0.2 running')

firmwareInfo()

-- configure for 9600, 8N1, with echo
-- uart.setup(0, 9600, 8, uart.PARITY_NONE, uart.STOPBITS_1, 1)
-- print('uart configured')

-- call abortInit after 1s
tmr.create():alarm(1000, tmr.ALARM_SINGLE, abortInit)
       
