SELECT name, level, nickname
FROM Gym, CatchedPokemon, Pokemon
WHERE Gym.leader_id = CatchedPokemon.owner_id
AND Pokemon.id = CatchedPokemon.pid
AND nickname LIKE 'A%'
ORDER BY name DESC