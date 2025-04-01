
CREATE TABLE `flag` (
  `flag_id` varchar(255) NOT NULL,
  `flag` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `flag`
  ADD UNIQUE KEY `flag_id` (`flag_id`);