SELECT name
FROM Pokemon, Evolution
WHERE Evolution.before_id = Pokemon.id
AND before_id > after_id
ORDER BY name