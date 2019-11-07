-- BAUD = 115200
-- DATABITS = 8
-- PARITY = uart.PARITY_NONE
-- STOPBITS = uart.STOPBITS_1

-- uart.setup(0, BAUD, DATABITS, PARITY, STOPBITS, 1)
-- uart.write(0, "UART 0 working for output\n")

-- function process(data)
--   print(data)
--   return
-- end


-- uart.on("data", "\n", process("run"), 1)

-- print("enter run or  abort")
-- process("quit")

-- return


-- when 4 chars is received.
uart.on("data", 4,
  function(data)
  print("receive from uart:", data)
  if data=="quit" then
    uart.on("data") -- unregister callback function
  end
end, 0)
-- when '\r' is received.
uart.on("data", "\r",
  function(data)
  print("receive from uart:", data)
  if data=="quit\r" then
    uart.on("data") -- unregister callback function
  end
end, 0)