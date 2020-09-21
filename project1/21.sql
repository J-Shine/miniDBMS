SELECT Trainer.name, COUNT(*)
FROM Gym, Trainer, CatchedPokemon
WHERE Gym.leader_id = Trainer.id
AND CatchedPokemon.owner_id = Gym.leader_id
GROUP BY Gym.leader_id
ORDER BY Trainer.name