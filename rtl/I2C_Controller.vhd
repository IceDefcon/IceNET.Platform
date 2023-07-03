library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity I2C_Controller is
    port (
        clk : in std_logic;
        rst : in std_logic;
        sda : inout std_logic;
        scl : inout std_logic
    );
end entity I2C_Controller;

architecture rtl of I2C_Controller is
    -- I2C states
    type StateType is (IDLE, START_BIT, DATA_BIT, ACK_BIT);
    signal state : StateType;
    
    -- Internal registers
    signal dataReg : std_logic_vector(7 downto 0);
    
begin

    I2C_Controller_Process : process(clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                state <= IDLE;
                dataReg <= (others => '0');
            else
                case state is
                    when IDLE =>
                        -- Check for start condition and initiate transmission
                        if (start_condition_detected) then
                            state <= START_BIT;
                        end if;

                    when START_BIT =>
                        -- Drive SDA low and release SCL
                        sda <= '0';
                        scl <= '1';
                        state <= DATA_BIT;

                    when DATA_BIT =>
                        -- Shift out data bit and wait for ACK
                        if (transmission_complete) then
                            state <= ACK_BIT;
                        end if;

                    when ACK_BIT =>
                        -- Handle ACK or NACK condition and transition to the next state
                        if (ack_received) then
                            -- ACK received, continue transmission
                            state <= DATA_BIT;
                        else
                            -- NACK received, transmission error
                            state <= IDLE;
                        end if;

                    when others =>
                        -- Handle other states if necessary
                        state <= IDLE;
                end case;
            end if;
        end if;
    end process I2C_Controller_Process;

    -- Connect the I2C controller signals to FPGA pins
    sda <= dataReg(7);
    scl <= (state = START_BIT) or (state = IDLE);

end architecture rtl;
