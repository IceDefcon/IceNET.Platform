	        if tx_current_state = DEVICE then
			    if i2c_clock = '1' then 	-- Align @ 1 to achieve cycle from '0'
			        i2c_clock_align <= '1'; -- Stay HI until reset
			    end if;

			    -- Clock is aligned
			    if i2c_clock_align = '1' then
			        if i2c_clock = '0' then  -- First '0' after aligned '1'

				    	i2c_clock_first <= '1';
				    	write_clock <= i2c_clock; 	-----===[ OUT HI ]===-----
				    	
			            -- End of DEVICE address transmission
			            if write_count = "1111" then
			            	if write_count_last = "11111001" then
				                write_count <= "0000";
				            	tx_next_state <= RESET;
				                isDEVICE <= '1';

				                i2c_clock_align <= '0'; -- Reset Alignment
					    		i2c_clock_first <= '0'; -- Reset clock first
					    		write_detect <= '0'; 	-- Reset clock detection
								write_clock <= '0'; 	-- Pull up after TX is complete

				                sm_run <= '0'; 			-- Hold State Machine
				            else
				            	write_count_last <= write_count_last + '1';
				            end if;
			            end if;

			            -- Write clock cycle is detected
						if write_detect = '0' then
							write_detect <= '1';
			                write_count <= write_count + '1';
			                if write_count = "1111" then
			                	i2c_clock_last <= '1';
			                end if;
						end if;
					elsif i2c_clock_first = '1' then
				    	write_clock <= i2c_clock;  		-----===[ OUT LO ]===-----
						write_detect <= '0';
			        end if;
			    end if;
		    end if;