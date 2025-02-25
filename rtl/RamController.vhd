    library IEEE;
    use IEEE.STD_LOGIC_1164.ALL;
    use IEEE.STD_LOGIC_ARITH.ALL;
    use IEEE.STD_LOGIC_UNSIGNED.ALL;

    entity RamController is
    Port
    (
        --CLOCK_266MHz : in  std_logic;
        CLOCK_133MHz : in  std_logic;
        RESET       : in  std_logic;

        --
        --  rising edge [0]
        --       |
        --       |        [1]        [2]
        --       V____      ____      ____
        --       |    |    |    |    |    |
        --   ____|    |____|    |____|    |____     133Mhz
        --
        --
        --     rising edge [0]
        --            |
        --            |        [1]       [2]
        --            V____      ____      ____
        --            |    |    |    |    |    |
        --        ____|    |____|    |____|    |____    133Mhz @ 180° Forward
        --

        A0           : out std_logic; -- Address Bus
        A1           : out std_logic;
        A2           : out std_logic;
        A3           : out std_logic;
        A4           : out std_logic;
        A5           : out std_logic;
        A6           : out std_logic;
        A7           : out std_logic;
        A8           : out std_logic;
        A9           : out std_logic;
        A10          : out std_logic;
        A11          : out std_logic;
        A12          : out std_logic;

        BA0          : out std_logic; -- Bank Address
        BA1          : out std_logic;

        CLK_SDRAM   : out std_logic;
        CKE         : out std_logic;
        CS          : out std_logic;
        RAS         : out std_logic;
        CAS         : out std_logic;
        WE          : out std_logic;

        DQ0         : inout std_logic; -- Data Bus
        DQ1         : inout std_logic;
        DQ2         : inout std_logic;
        DQ3         : inout std_logic;
        DQ4         : inout std_logic;
        DQ5         : inout std_logic;
        DQ6         : inout std_logic;
        DQ7         : inout std_logic;
        DQ8         : inout std_logic;
        DQ9         : inout std_logic;
        DQ10        : inout std_logic;
        DQ11        : inout std_logic;
        DQ12        : inout std_logic;
        DQ13        : inout std_logic;
        DQ14        : inout std_logic;
        DQ15        : inout std_logic;

        LDQM        : out std_logic;
        UDQM        : out std_logic;

        -- User Interface
        ADDR        : in  std_logic_vector(23 downto 0);
        DATA_IN     : in  std_logic_vector(15 downto 0);
        DATA_OUT    : out std_logic_vector(15 downto 0);
        READ_EN     : in  std_logic;
        WRITE_EN    : in  std_logic;
        BUSY        : out std_logic
    );
    end RamController;

    architecture rtl of RamController is

        type SDRAM_STATE is
        (
            SDRAM_INIT,
            SDRAM_PRECHARGE,
            SDRAM_AUTO_REFRESH,
            SDRAM_MODE,
            SDRAM_IDLE,
            SDRAM_ACTIVE,
            SDRAM_WRITE_CMD,
            SDRAM_READ_CMD,
            SDRAM_READ_DATA,
            SDRAM_REFRESH,
            SDRAM_DONE
        );
        signal memory_state : SDRAM_STATE := SDRAM_INIT;

        -- Internal signals
        signal command : std_logic_vector(4 downto 0);
        signal process_read : std_logic := '0';
        signal process_write : std_logic := '0';

        signal init_counter : integer range 0 to 13300 := 0;
        signal refresh_counter : integer range 0 to 8192 := 0;
        signal AR_counter : integer range 0 to 2 := 0;
        signal tRP_counter : integer range 0 to 16 := 0;
        signal tMRD_counter : integer range 0 to 16 := 0;
        signal tRRC_counter : integer range 0 to 16 := 0;
        signal tRCD_counter : integer range 0 to 16 := 0;
        signal tRAS_counter : integer range 0 to 16 := 0;
        signal tCL_counter : integer range 0 to 16 := 0;

        constant CMD_INIT       : std_logic_vector(4 downto 0) := "01111"; -- 0x0F CKE DOWN :: INIT
        constant CMD_NOP        : std_logic_vector(4 downto 0) := "10111"; -- 0x17 NOP
        constant CMD_ACTIVE     : std_logic_vector(4 downto 0) := "10011"; -- 0x13 BANK ACTIVE
        constant CMD_WRITE      : std_logic_vector(4 downto 0) := "10100"; -- 0x14 WRITE
        constant CMD_READ       : std_logic_vector(4 downto 0) := "10101"; -- 0x15 READ
        constant CMD_BSTOP      : std_logic_vector(4 downto 0) := "10110"; -- 0x16 BURST STOP
        constant CMD_PRECHARGE  : std_logic_vector(4 downto 0) := "10010"; -- 0x12 PRECHARGE
        constant CMD_AUTOREFRESH: std_logic_vector(4 downto 0) := "10001"; -- 0x11 AUTO REFRESH
        constant CMD_MODE       : std_logic_vector(4 downto 0) := "10000"; -- 0x10 MODE REGISTER SET

        constant MODE_REG : std_logic_vector(12 downto 0) := "00" & "00" & "00" & "010" & "0" & "000";

        -- Registers for pipelining
        signal A0_reg, A1_reg, A2_reg, A3_reg, A4_reg, A5_reg, A6_reg, A7_reg, A8_reg, A9_reg, A10_reg, A11_reg, A12_reg : std_logic;
        signal command_reg : std_logic_vector(4 downto 0);

    begin

        process (CLOCK_133MHz)
        begin
            if rising_edge(CLOCK_133MHz) then
                CLK_SDRAM <= not CLOCK_133MHz;
                CKE <= command(4);
                CS  <= command(3);
                RAS <= command(2);
                CAS <= command(1);
                WE  <= command(0);
                LDQM <= '0';
                UDQM <= '0';
            end if;
        end process;

        process (CLOCK_133MHz, RESET)
        begin
            if RESET = '1' then

                init_counter <= 0;
                refresh_counter <= 0;
                process_read <= '0';
                process_write <= '0';

                command <= CMD_INIT;
                memory_state <= SDRAM_INIT;

                AR_counter <= 1; -- Interrations of auto-refresh

            elsif rising_edge(CLOCK_133MHz) then
                case memory_state is

                    when SDRAM_INIT =>
                        command <= CMD_NOP;
                        if init_counter = 13300 then
                            memory_state <= SDRAM_PRECHARGE;
                        else
                            init_counter <= init_counter + 1;
                        end if;
                        A0 <= '0';
                        A1 <= '0';
                        A2 <= '0';
                        A3 <= '0';
                        A4 <= '0';
                        A5 <= '0';
                        A6 <= '0';
                        A7 <= '0';
                        A8 <= '0';
                        A9 <= '0';
                        A10 <= '0';
                        A11 <= '0';
                        A12 <= '0';
                        BA0 <= '0';
                        BA1 <= '0';
                        DATA_OUT <= (others => '0');

                    when SDRAM_PRECHARGE =>
                        command <= CMD_PRECHARGE;
                        --A <= (others => '1');
                        A0 <= '1';
                        A1 <= '1';
                        A2 <= '1';
                        A3 <= '1';
                        A4 <= '1';
                        A5 <= '1';
                        A6 <= '1';
                        A7 <= '1';
                        A8 <= '1';
                        A9 <= '1';
                        A10 <= '1';
                        A11 <= '1';
                        A12 <= '1';

                        BA0 <= '1';
                        BA1 <= '1';

                        memory_state <= SDRAM_AUTO_REFRESH;
                        tRP_counter <= 3;  -- 20ns / (1/133MHz) = 2.660 cycles

                    when SDRAM_AUTO_REFRESH =>
                        if tRP_counter = 0 then
                            command <= CMD_AUTOREFRESH;
                            memory_state <= SDRAM_MODE;
                            tRRC_counter <= 9; -- 63ns / (1/133MHz) = 0.266 cycles
                        else
                            command <= CMD_NOP;
                            tRP_counter <= tRP_counter - 1;
                        end if;

                    when SDRAM_MODE =>
                        if tRRC_counter = 0 then
                            if AR_counter = 0 then
                                A0 <= MODE_REG(0);
                                A1 <= MODE_REG(1);
                                A2 <= MODE_REG(2);
                                A3 <= MODE_REG(3);
                                A4 <= MODE_REG(4);
                                A5 <= MODE_REG(5);
                                A6 <= MODE_REG(6);
                                A7 <= MODE_REG(7);
                                A8 <= MODE_REG(8);
                                A9 <= MODE_REG(9);
                                A10 <= MODE_REG(10);
                                A11 <= MODE_REG(11);
                                A12 <= MODE_REG(12);

                                BA0 <= '0';
                                BA1 <= '0';

                                command <= CMD_MODE;
                                memory_state <= SDRAM_IDLE;
                                tMRD_counter <= 1; -- 2ns  / (1/133MHz) = 2.660 cycles
                             else
                                command <= CMD_NOP;
                                memory_state <= SDRAM_AUTO_REFRESH;
                                AR_counter <= AR_counter - 1;
                            end if;
                        else
                            command <= CMD_NOP;
                            tRRC_counter <= tRRC_counter - 1;
                        end if;

                    when SDRAM_IDLE =>
                        command <= CMD_NOP;
                        if tMRD_counter = 0 then

                            if WRITE_EN = '1' then
                                process_write <= '1';
                                memory_state <= SDRAM_ACTIVE;
                            elsif READ_EN = '1' then
                                process_read <= '1';
                                memory_state <= SDRAM_ACTIVE;
                            elsif refresh_counter = 8192 then
                                memory_state <= SDRAM_REFRESH;
                            end if;

                        else
                            tMRD_counter <= tMRD_counter - 1;
                        end if;

                    -----------------------------------------------
                    --
                    -- A(12 downto 0) :: ADDR(23 downto 11) :: Row
                    -- BA(1 downo 0) :: ADDR(10 downto 9) :: Bank
                    -- A(8 downto 0) :: ADDR(8 downto 0) :: Column
                    -- A(10) ::

                    -- SD_ADDRESS
                    -- SD_BANK
                    when SDRAM_ACTIVE =>
                        command <= CMD_ACTIVE;
                        --A <= ADDR(23 downto 11);
                        A0 <= ADDR(11);
                        A1 <= ADDR(12);
                        A2 <= ADDR(13);
                        A3 <= ADDR(14);
                        A4 <= ADDR(15);
                        A5 <= ADDR(16);
                        A6 <= ADDR(17);
                        A7 <= ADDR(18);
                        A8 <= ADDR(19);
                        A9 <= ADDR(20);
                        A10 <= ADDR(21);
                        A11 <= ADDR(22);
                        A12 <= ADDR(23);

                        BA0 <= ADDR(9);
                        BA1 <= ADDR(10);

                        if process_write = '1' then
                            memory_state <= SDRAM_WRITE_CMD;
                        elsif process_read = '1' then
                            memory_state <= SDRAM_READ_CMD;
                        end if;
                        tRCD_counter <= 3; -- 20ns / (1/133MHz) = 2.660 cycles

                    when SDRAM_WRITE_CMD =>
                        if tRCD_counter = 0 then
                            command <= CMD_WRITE;

                            A0 <= ADDR(0); -- Column
                            A1 <= ADDR(1);
                            A2 <= ADDR(2);
                            A3 <= ADDR(3);
                            A4 <= ADDR(4);
                            A5 <= ADDR(5);
                            A6 <= ADDR(6);
                            A7 <= ADDR(7);
                            A8 <= ADDR(8);
                            A9 <= '0';
                            A10 <= '1'; -- Precharge
                            A11 <= '0';
                            A12 <= '0';

                            DQ0 <= DATA_IN(0);
                            DQ1 <= DATA_IN(1);
                            DQ2 <= DATA_IN(2);
                            DQ3 <= DATA_IN(3);
                            DQ4 <= DATA_IN(4);
                            DQ5 <= DATA_IN(5);
                            DQ6 <= DATA_IN(6);
                            DQ7 <= DATA_IN(7);
                            DQ8 <= DATA_IN(8);
                            DQ9 <= DATA_IN(9);
                            DQ10 <= DATA_IN(10);
                            DQ11 <= DATA_IN(11);
                            DQ12 <= DATA_IN(12);
                            DQ13 <= DATA_IN(13);
                            DQ14 <= DATA_IN(14);
                            DQ15 <= DATA_IN(15);

                            memory_state <= SDRAM_DONE;
                            tRAS_counter <= 6; -- 42ns / (1/133Mhz) = 5.586 cycles
                        else
                            command <= CMD_NOP;
                            tRCD_counter <= tRCD_counter - 1;
                        end if;

                    when SDRAM_READ_CMD =>
                        if tRCD_counter = 0 then
                            command <= CMD_READ;

                            A0 <= ADDR(0); -- Column
                            A1 <= ADDR(1);
                            A2 <= ADDR(2);
                            A3 <= ADDR(3);
                            A4 <= ADDR(4);
                            A5 <= ADDR(5);
                            A6 <= ADDR(6);
                            A7 <= ADDR(7);
                            A8 <= ADDR(8);
                            A9 <= '0';
                            A10 <= '1'; -- Precharge
                            A11 <= '0';
                            A12 <= '0';

                            DQ0 <= 'Z';
                            DQ1 <= 'Z';
                            DQ2 <= 'Z';
                            DQ3 <= 'Z';
                            DQ4 <= 'Z';
                            DQ5 <= 'Z';
                            DQ6 <= 'Z';
                            DQ7 <= 'Z';
                            DQ8 <= 'Z';
                            DQ9 <= 'Z';
                            DQ10 <= 'Z';
                            DQ11 <= 'Z';
                            DQ12 <= 'Z';
                            DQ13 <= 'Z';
                            DQ14 <= 'Z';
                            DQ15 <= 'Z';

                            memory_state <= SDRAM_READ_DATA;
                            tCL_counter <= 3;  -- For 133MHz Clock :: tCL × Clock Period = 3×7.5ns = 22.5ns
                        else
                            command <= CMD_NOP;
                            tRCD_counter <= tRCD_counter - 1;
                        end if;

                    when SDRAM_READ_DATA =>
                        command <= CMD_NOP;
                        if tCL_counter = 0 then
                            DATA_OUT(0) <= DQ0;
                            DATA_OUT(1) <= DQ1;
                            DATA_OUT(2) <= DQ2;
                            DATA_OUT(3) <= DQ3;
                            DATA_OUT(4) <= DQ4;
                            DATA_OUT(5) <= DQ5;
                            DATA_OUT(6) <= DQ6;
                            DATA_OUT(7) <= DQ7;
                            DATA_OUT(8) <= DQ8;
                            DATA_OUT(9) <= DQ9;
                            DATA_OUT(10) <= DQ10;
                            DATA_OUT(11) <= DQ11;
                            DATA_OUT(12) <= DQ12;
                            DATA_OUT(13) <= DQ13;
                            DATA_OUT(14) <= DQ14;
                            DATA_OUT(15) <= DQ15;
                            memory_state <= SDRAM_DONE;
                            tRAS_counter <= 6; -- 42ns / (1/133Mhz) = 5.586 cycles
                        else
                            tCL_counter <= tCL_counter - 1;
                        end if;

                    when SDRAM_REFRESH =>
                        command <= CMD_AUTOREFRESH;
                        refresh_counter <= 0;
                        memory_state <= SDRAM_IDLE;
                        tMRD_counter <= 1; -- 2ns  / (1/133MHz) = 2.660 cycles

                    when SDRAM_DONE =>
                        command <= CMD_NOP;
                        if tRAS_counter = 0 then
                            process_read <= '0';
                            process_write <= '0';
                            memory_state <= SDRAM_IDLE;
                            tMRD_counter <= 1; -- 2ns  / (1/133MHz) = 2.660 cycles
                        else
                            tRAS_counter <= tRAS_counter - 1;
                        end if;
                end case;

                refresh_counter <= refresh_counter + 1;
            end if;
        end process;

        BUSY <= '1' when memory_state /= SDRAM_IDLE else '0';
    end rtl;
