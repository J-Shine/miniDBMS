SELECT AVG(level)
FROM Gym, CatchedPokemon
WHERE Gym.leader_id = CatchedPokemon.owner_id