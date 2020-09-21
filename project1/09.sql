SELECT name, AVG(level)
FROM Gym, Trainer, CatchedPokemon
WHERE Gym.leader_id = Trainer.id
AND CatchedPokemon.owner_id = Trainer.id
GROUP BY name
ORDER BY name