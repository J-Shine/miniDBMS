SELECT COUNT(DISTINCT type)
FROM Gym, CatchedPokemon, Pokemon
WHERE Gym.leader_id = CatchedPokemon.owner_id
AND Pokemon.id = CatchedPokemon.pid
AND Gym.city = 'Sangnok City'